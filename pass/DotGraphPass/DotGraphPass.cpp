#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/CFG.h"

#include <ostream>
#include <sstream>
#include <string.h>
#include <list>
#include <unordered_map>
#include <cassert>
#include <fstream>
#include <stdexcept>


using namespace llvm;

namespace {

struct DotGraphPass : PassInfoMixin<DotGraphPass> {

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {

        uint64_t id = 0;

        std::string digraph = GetDigraph(M);
        WriteOutFile("./output_artifacts/DotGraphPass.dot", digraph);

        return PreservedAnalyses::all();
    }

    std::string GetDigraph(llvm::Module& module) {
        std::string ret = "digraph Program{\n";

        for (Function & F: module) {
            ret += FuncToDigraph(F);
        }

        ret += GetFunctionCallGraph(module);


        ret += "}\n";
        return ret;
    }

    std::string LLVMInstToString(llvm::Instruction &inst) {
        std::string str;
        llvm::raw_string_ostream rso(str);

        inst.print(rso);

        rso.flush();
        return str;
    }


    std::string GetFunctionCallGraph(llvm::Module& M) {
        std::string ret = "";
        for (Function &F : M) {
            if (F.isDeclaration()) continue;

            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {

                    if (auto *CI = dyn_cast<CallInst>(&I)) {
                        if (Function *Callee = CI->getCalledFunction()) {

                            if (!Callee->isDeclaration()) {
                                ret += "\t" + F.getName().str() +
                                    " -> " +
                                    Callee->getName().str() + ";\n";
                            }
                        }
                    }
                }
            }
        }
        return ret;
    }


    std::string FuncToDigraph(llvm::Function& func) {
        std::string ret = "\tsubgraph cluster_" + func.getName().str() + "{\n";
        // anchor node
        ret += "\t\t" + func.getName().str()  + " [shape=box];\n";
        for (BasicBlock &BB : func) {
            NameBasicBlock(&BB);
            ret += BBToDigraph(BB);
        }

        for (BasicBlock &BB : func) {
            for (BasicBlock* successor : successors(&BB)) {
                ret += "\t" + GetBasicBlockName(&BB) + "->" + GetBasicBlockName(successor) + ";\n";
            }
        }


        ret += "\t}\n";


        return ret;
    }

    std::string BBToDigraph(llvm::BasicBlock& bb) {
        std::string ret = "\t\t" + GetBasicBlockName(&bb);

        std::string instructions = "";
        for (llvm::Instruction& inst: bb) {
            instructions += LLVMInstToString(inst) + "\n";
        }

        ret += " [label=\"" + instructions +"\"];\n";
        return ret;
    }


    void WriteOutFile(const std::string& file, const std::string& contents) {
        std::ofstream outfile(file, std::ios::out | std::ios::trunc);

        if (!outfile) {
            throw std::runtime_error("Failed to open file: " + file);
        }

        outfile << contents;

        if (!outfile) {
            throw std::runtime_error("Failed to write to file: " + file);
        }
    }

    std::string GetBasicBlockName(llvm::BasicBlock* bb) {
        assert(m_BBToID.find(bb) != m_BBToID.end());
        return "BB" + std::to_string(m_BBToID.at(bb));
    }

    void NameBasicBlock(llvm::BasicBlock* bb) {
        m_BBToID.insert({bb, m_BBid++});
    }


    int m_BBid = 0;
    std::unordered_map<llvm::BasicBlock*, int> m_BBToID;
    std::unordered_map<std::string, std::string> m_FuncCFG;

    std::string dotFileString;
    std::string outfile = "output_artifacts/DotGraphPass.dot";
};

} // namespace


// REQUIRED LLVM 14 plugin entry point
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(void) {

    return {
        LLVM_PLUGIN_API_VERSION,
        "DotGraphPass",
        LLVM_VERSION_STRING,

        // registration callback
        [](PassBuilder &PB) {

            PB.registerPipelineParsingCallback(
                [](StringRef Name,
                   ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {

                    if (Name == "DotGraphPass") {
                        MPM.addPass(DotGraphPass());
                        return true;
                    }

                    return false;
                });
        }
    };
}