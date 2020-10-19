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
#include <system_error>
#include <utility>
#include <vector>


#include "defs_ast.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"


static void HandleDefinition(Parser * parser) {

  if (auto FnAST = parser->parseDef()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read function definition:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  } else parser->eat();
}

static void HandleStruct(Parser * parser) {

  if (auto FnAST = parser->parseStruct()) {
    if (auto *FnIR = (llvm::Type*)FnAST->codegen()) {
      fprintf(stderr, "Read struct definition:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  } else parser->eat();
}


static void HandleExtern(Parser * parser) {
  if (auto ProtoAST = parser->parseExtern()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern: ");
      FnIR->print(errs());
      fprintf(stderr, "\n");
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

int main() {

  // Nothing to see here... for a while

  return 0;
}