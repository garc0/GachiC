#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"


#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <fstream>

#include <system_error>
#include <utility>
#include <vector>


#include "defs_ast.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

#include "Utils/clipp.h"

static void HandleDefinition(Parser * parser) {

  if (auto FnAST = parser->parseDef()) {
    if (auto *FnIR = FnAST->codegen()) {
      FnIR->print(errs());
    }
  } else parser->eat();
}

static void HandleMainDef(Parser * parser) {

  if (auto FnAST = parser->parseMain()) {
    if (auto *FnIR = FnAST->codegen()) {
      FnIR->print(errs());
    }
  } else parser->eat();
}

static void HandleStruct(Parser * parser) {

  if (auto FnAST = parser->parseStruct()) {
    if (auto *FnIR = (llvm::Type*)FnAST->codegen()) {
      FnIR->print(errs());
    }
  } else parser->eat();
}

static void HandleExtern(Parser * parser) {
  if (auto ProtoAST = parser->parseExtern()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      FnIR->print(errs());
      FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else parser->eat();
}


static void HandleTopLevelExpression(Parser * parser) {
  if (auto FnAST = parser->parseTopLevelExpr()) {
    FnAST->codegen();
  } else parser->eat();
}

static void MainLoop(Parser * parser) {
  while (true) {
    switch ((parser->getCurrentToken()).kind()) {
    case Token::Kind::End:
      return;
    case Token::Kind::Semicolon:
      parser->eat();
      break;
    case Token::Kind::Def:
      HandleDefinition(parser);
      break;
    case Token::Kind::Master:
      HandleMainDef(parser);
      break;
    case Token::Kind::Extern:
      HandleExtern(parser);
      break;
    case Token::Kind::Struct:
      HandleStruct(parser);
      break;
    default:
      HandleTopLevelExpression(parser);
      break;
    }
  }
}

void dump_tokens(Lexer * lex){

  Token tok = lex->get();
  while(tok.is_kind(Token::Kind::End)){
    std::cout << tok.kind() << " (" << tok.lexeme() << ") \n";
  }
  lex->to_begin();

  return;
}

void dump_ast(Parser * parser){

  std::cerr << "AST dump is currently not supported" << std::endl;

  return;
}

void dump_ir(){

  std::cerr << "IR dump is currently not supported" << std::endl;

  return;
}

int write_object_file(std::string o_filename, std::unique_ptr<llvm::Module> o_Module){
   
  using namespace llvm;

  // Initialize the target registry etc.
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  auto TargetTriple = sys::getDefaultTargetTriple();
  o_Module->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    errs() << Error;
    return -1;
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  auto RM = Optional<Reloc::Model>();
  auto TheTargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  o_Module->setDataLayout(TheTargetMachine->createDataLayout());

  std::error_code EC;
  raw_fd_ostream dest(o_filename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << "Could not open file: " << EC.message();
    return -1;
  }

  legacy::PassManager pass;
  auto FileType = CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    errs() << "TheTargetMachine can't emit a file of this type";
    return -1;
  }

  pass.run(*o_Module);
  dest.flush();

  return 0;
}

int main(int argc, char* argv[]) {

  bool  to_dump = false,

        d_token = false,
        d_ast   = false,
        d_ir    = false;

  std::vector<std::string> filenames;

  {
    using namespace clipp;

    auto cli = (
      option("-d", "--dump").set(to_dump) & 
        (
          required("tokens").set(d_token) |
          required("ast").set(d_ast) //|
          //required("ir").set(d_ir)
        ),
      option("-c", "--compile") & values("filenames", filenames) 
    );

    if(!parse(argc, argv, cli)){
      std::cout << clipp::make_man_page(cli, argv[0]);
      return -1;
    }

  }

  if(filenames.size() == 0){
    std::cerr << "i need a source files!" << std::endl;
    return -1;
  }

  for(auto &module_name : filenames){

    auto lex = std::make_unique<Lexer>();

    {
      std::ifstream in(module_name);
        
      if(in.is_open()){
        std::string line;

          while(getline(in, line))
            lex->analyze(line);

        lex->set_end();

        in.close();
      }else{
        std::cout << "cannot open input file" << std::endl;
        return -1;
      }
    }

    if(to_dump && d_token) dump_tokens(lex.get());

    auto parser = std::make_unique<Parser>(std::move(lex));

    if(to_dump && d_ast) dump_ast(parser.get()); 

    if(to_dump && d_ir) dump_ir(); 

    {
      FunctionProtos.clear();

      StructFields.clear();

      NamedValues.clear();
      NamedStructures.clear();
      
      TheModule = std::make_unique<Module>(module_name, TheContext);

      MainLoop(parser.get());

      std::size_t dot_pos = module_name.find_last_of('.'); 

      if(write_object_file(module_name.substr(0, dot_pos) + ".o", std::move(TheModule)) != 0)
        return -1;
    }
  }
  
  return 0;
}