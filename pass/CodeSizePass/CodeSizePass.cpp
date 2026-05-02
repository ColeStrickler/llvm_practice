#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

namespace {

struct CodeSizePass : PassInfoMixin<CodeSizePass> {

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {

        uint64_t id = 0;
        uint64_t total_code_size = 0;

        for (Function &F : M) {
            if (F.isDeclaration())
                continue;

            errs() << "Function: " << F.getName() << "\n";
            uint64_t function_code_size = 0;
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                   function_code_size++;
                }
            }
             errs() << "Function Code Size:" << function_code_size << "\n\n";
            total_code_size += function_code_size;
        }
        errs() << "Total Code Size:" << total_code_size << "\n\n";

        return PreservedAnalyses::all();
    }
};

} // namespace


// REQUIRED LLVM 14 plugin entry point
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(void) {

    return {
        LLVM_PLUGIN_API_VERSION,
        "CodeSizePass",
        LLVM_VERSION_STRING,

        // registration callback
        [](PassBuilder &PB) {

            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                   ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {

                    if (Name == "CodeSizePass") {
                        MPM.addPass(CodeSizePass());
                        return true;
                    }

                    return false;
                });
        }
    };
}