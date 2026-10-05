#pragma once
#include <vector>
#include <functional>

namespace rm {
    template<typename... Args>
    class Signal {
        using Signature = std::function<bool(Args...)>;

    public:
        void Register(Signature fn) {
            listeners.push_back(std::move(fn));
        }

        void Dispatch(Args... args) const {
            for (auto& cb : listeners) {
                if (cb(args...)) {
                    break;
                }
            }
        }
    private:
        std::vector<Signature> listeners;
    };
}
