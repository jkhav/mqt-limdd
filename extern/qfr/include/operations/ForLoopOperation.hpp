/*
 * This file is part of MQT QFR library which is released under the MIT license.
 * See file README.md or go to https://www.cda.cit.tum.de/research/quantum/ for more information.
 */

#pragma once

#include "Operation.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace qc {

    class ForLoopOperation final: public Operation {
    protected:
        int                                     start = 0;
        int                                     stop  = 0;
        std::string                             loopVar{};
        std::vector<std::unique_ptr<Operation>> body{};

    public:
        // Called from QASMParser.cpp via QuantumComputation::emplace_back<ForLoopOperation>(...)
        ForLoopOperation(int start, int stop, std::string loopVar, std::vector<std::unique_ptr<Operation>> body):
            start(start), stop(stop), loopVar(std::move(loopVar)), body(std::move(body)) {
            std::strcpy(name, "for");
            type = ForLoop;
            parameter[0] = static_cast<dd::fp>(start);
            parameter[1] = static_cast<dd::fp>(stop);
        }

        [[nodiscard]] std::unique_ptr<Operation> clone() const override {
            std::vector<std::unique_ptr<Operation>> clonedBody{};
            clonedBody.reserve(body.size());
            for (const auto& op: body) {
                clonedBody.emplace_back(op->clone());
            }
            return std::make_unique<ForLoopOperation>(start, stop, loopVar, std::move(clonedBody));
        }

        void setNqubits(dd::QubitCount nq) override {
            nqubits = nq;
            for (auto& op: body) {
                op->setNqubits(nq);
            }
        }

        [[nodiscard]] bool isUnitary() const override {
            return false;
        }

        [[nodiscard]] int getStart() const {
            return start;
        }

        [[nodiscard]] int getStop() const {
            return stop;
        }

        [[nodiscard]] const std::string& getLoopVar() const {
            return loopVar;
        }

        [[nodiscard]] const std::vector<std::unique_ptr<Operation>>& getBody() const {
            return body;
        }

        std::vector<std::unique_ptr<Operation>>& getBody() {
            return body;
        }

        [[nodiscard]] bool equals(const Operation& op, const Permutation& perm1, const Permutation& perm2) const override {
            if (const auto* loop = dynamic_cast<const ForLoopOperation*>(&op)) {
                if (start != loop->start || stop != loop->stop || loopVar != loop->loopVar || body.size() != loop->body.size()) {
                    return false;
                }
                for (std::size_t i = 0; i < body.size(); ++i) {
                    if (!body[i]->equals(*loop->body[i], perm1, perm2)) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        [[nodiscard]] bool equals(const Operation& operation) const override {
            return equals(operation, {}, {});
        }

        std::ostream& print(std::ostream& os) const override {
            os << "for int " << loopVar << " in [" << start << ":" << stop << "] {";
            for (const auto& op: body) {
                os << std::endl
                   << "\t";
                op->print(os);
            }
            os << std::endl
               << "}";
            return os;
        }

        std::ostream& print(std::ostream& os, const Permutation& permutation) const override {
            os << "for int " << loopVar << " in [" << start << ":" << stop << "] {";
            for (const auto& op: body) {
                os << std::endl
                   << "\t";
                op->print(os, permutation);
            }
            os << std::endl
               << "}";
            return os;
        }

        [[nodiscard]] bool actsOn(dd::Qubit i) const override {
            return std::any_of(body.cbegin(), body.cend(), [&i](const auto& op) { return op->actsOn(i); });
        }

        [[nodiscard]] std::set<dd::Qubit> getUsedQubits() const override {
            std::set<dd::Qubit> usedQubits{};
            for (const auto& op: body) {
                usedQubits.merge(op->getUsedQubits());
            }
            return usedQubits;
        }

        void dumpOpenQASM(std::ostream& of, const RegisterNames& qreg, const RegisterNames& creg) const override {
            of << "for int " << loopVar << " in [" << start << ":" << stop << "] {\n";
            for (const auto& op: body) {
                op->dumpOpenQASM(of, qreg, creg);
            }
            of << "}\n";
        }

        void dumpQiskit([[maybe_unused]] std::ostream& of, [[maybe_unused]] const RegisterNames& qreg, [[maybe_unused]] const RegisterNames& creg, [[maybe_unused]] const char* anc_reg_name) const override {
            throw QFRException("Dumping of for-loops currently not supported for qiskit");
        }
    };
} // namespace qc
