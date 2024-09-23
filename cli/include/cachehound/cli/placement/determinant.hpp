#ifndef CACHEHOUND_CLI_PLACEMENT_DETERMINANT_HPP
#define CACHEHOUND_CLI_PLACEMENT_DETERMINANT_HPP

#include <Eigen/Eigen>
#include <Eigen/LU>
#include <boost/multiprecision/fwd.hpp>
#include <cmath>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/eigen.hpp>

#include "./bit.hpp"

namespace cachehound::cli {

template<typename T, auto... MatrixArgs>
T determinant(Eigen::Matrix<T, MatrixArgs...>& matrix) {
    assert(matrix.rows() == matrix.cols());
    assert(matrix.rows() > 1);

    // if(matrix.rows() > 12) {
    //     // if(matrix.rows() == 2) {
    //     //     return matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0);
    //     // }

    //     Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> sub_matrix;
    //     sub_matrix = matrix.block(1, 1, matrix.rows() - 1, matrix.cols() - 1);

    //     T det = matrix(0, 0) * determinant(sub_matrix);
    //     bool sub = true;
    //     for(std::size_t col = 1; col < matrix.cols(); col++) {
    //         auto x = matrix(0, col);
    //         sub_matrix << matrix.block(1, 0, matrix.rows() - 1, col)
    //                     , matrix.block(1, col + 1, matrix.rows() - 1, matrix.cols() - col - 1);
    //         x *= determinant(sub_matrix);

    //         if(sub) {
    //             det -= x;
    //         } else {
    //             det += x;
    //         }

    //         sub = !sub;
    //     }
    //     return det;
    // }

    if constexpr(std::same_as<T, bit>) {
        using namespace boost::multiprecision;

        Eigen::Matrix<mpq_rational, MatrixArgs...> copy(matrix.rows(), matrix.cols());
        for(int r = 0; r < matrix.rows(); r++) {
            for(int c = 0; c < matrix.cols(); c++) {
                bool value = matrix(r, c);
                copy(r, c) = value ? 1 : 0;
            }
        }
        mpq_rational d = copy.determinant();
        auto i = d.convert_to<mpz_int>();
        i %= 2;
        return bit{!i.is_zero()};
    } else {
        return matrix.determinant();
    }
}

}


#endif /* CACHEHOUND_CLI_PLACEMENT_DETERMINANT_HPP */
