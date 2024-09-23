#ifndef CACHEHOUND_CLI_PLACEMENT_AFFINE_SOLVER_HPP
#define CACHEHOUND_CLI_PLACEMENT_AFFINE_SOLVER_HPP

#include <Eigen/Eigen>
#include "./determinant.hpp"
#include "./stack.hpp"

namespace cachehound::cli {

template<typename ScalarType>
class affine_solver {
    std::size_t input_dimension_
              , output_dimension_;

public:
    using matrix_type = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>;
    using vector_type = Eigen::Vector<ScalarType, Eigen::Dynamic>;

    affine_solver(std::size_t input_dimension, std::size_t output_dimension)
        : input_dimension_(input_dimension)
        , output_dimension_(output_dimension) {}

    std::pair<matrix_type, vector_type> solve(auto&& inputs, auto&& outputs) {
        matrix_type input_matrix = hstack<ScalarType>(std::forward<decltype(inputs)>(inputs));
        matrix_type output_matrix = hstack<ScalarType>(std::forward<decltype(outputs)>(outputs));

        matrix_type B(input_matrix.rows() + 1, input_matrix.cols());
        B.topRows(input_matrix.rows()) = input_matrix;
        B.bottomRows(1) = matrix_type::Ones(1, input_matrix.cols());

        auto D = determinant(B);
        assert(D != ScalarType{}); // If this fails, the system can not be solved

        auto entry = [&](const auto& r, int d){
            matrix_type temp(B.rows(), B.cols());
            temp << r, B.topRows(d), B.bottomRows(B.rows() - d - 1);
            return determinant(temp);
        };

        matrix_type M(outputs.front().size(), outputs.size());
        for (int i = 0; i < M.rows(); ++i) {
            const auto& R = output_matrix.row(i);
            for (int j = 0; j < M.cols(); ++j) {
                M(i, j) = entry(R, j);
            }
        }

        matrix_type A = M.leftCols(inputs.size() - 1);
        vector_type t = M.rightCols(1);
        return {A, t};
    }
};

}

#endif /* CACHEHOUND_CLI_PLACEMENT_AFFINE_SOLVER_HPP */
