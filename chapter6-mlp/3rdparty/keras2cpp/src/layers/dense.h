#pragma once
#include "activation.h"
namespace keras2cpp{
    namespace layers{
        class Dense final : public Layer<Dense> {
        public:
            Tensor weights_;
            Tensor biases_;
            Activation activation_;

            Dense(Stream& file);
            Tensor operator()(const Tensor& in) const noexcept override;
        };
    }
}
