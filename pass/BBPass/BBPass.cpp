#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

namespace {

struct BBPass : PassInfoMixin<BBPass> {

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {

        uint64_t id = 0;

        for (Function &F : M) {
            if (F.isDeclaration())
                continue;

            errs() << "Function: " << F.getName() << "\n";

            for (BasicBlock &BB : F) {
                errs() << "  BB " << id << "\n";
                id++;
            }
        }

        return PreservedAnalyses::all();
    }
};

} // namespace


// REQUIRED LLVM 14 plugin entry point
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(void) {

    return {
        LLVM_PLUGIN_API_VERSION,
        "BBPass",
        LLVM_VERSION_STRING,

        // registration callback
        [](PassBuilder &PB) {

            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                   ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {

                    if (Name == "BBPass") {
                        MPM.addPass(BBPass());
                        return true;
                    }

                    return false;
                });
        }
    };
}