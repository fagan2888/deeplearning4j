/*******************************************************************************
 * Copyright (c) 2015-2018 Skymind, Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// This file contains operations added by 3rd parties
//
// @author raver119@gmail.com
//

#include <system/op_boilerplate.h>
#if NOT_EXCLUDED(OP_firas_sparse)

#ifndef LIBND4J_THIRD_PARTY_H
#define LIBND4J_THIRD_PARTY_H

#include <system/op_boilerplate.h>
#include <memory>
#include <helpers/shape.h>
#include <loops/random.h>
#include <array/NDArray.h>
#include <ops/declarable/DeclarableOp.h>
#include <ops/declarable/OpRegistrator.h>
#include <ops/declarable/CustomOperations.h>

namespace sd {
    namespace ops {


        /**
         * This op is special one, and suited only for ProjectionLayer by @firasdib
         *
         *
         *
         * @tparam T
         */
        CUSTOM_OP_IMPL(firas_sparse, 1, 1, false, 0, -1) {
            auto x = INPUT_VARIABLE(0);
            auto z = OUTPUT_NULLIFIED(0);

            int batchSize = x->sizeAt(0);
            int numColumns = x->sizeAt(1);

            std::vector<int> indices(*block.getIArguments());
            std::map<int, int> sparse2dense;


            int cnt = 0;
            for (auto v: indices) {
                std::pair<int, int> pair(v, cnt++);
                sparse2dense.insert(pair);
            }

            ResultSet rows = x->allTensorsAlongDimension({1});

            //PRAGMA_OMP_PARALLEL_FOR
            for (int r = 0; r < batchSize; r++) {
                auto row = rows.at(r);

                for (int e = 0; e < numColumns; e += 2) {
                    int idx = row->e<int>(e);
                    if (idx < 0)
                        break;

                    int denseIdx = sparse2dense.at(idx);


                    float value = row->e<float>(e);
                    float current = z->e<float>(r, denseIdx);
                    z->p(r, denseIdx, value + current);
                }
            }


            //STORE_RESULT(*z);

            return Status::OK();
        }

        DECLARE_SHAPE_FN(firas_sparse) {
            auto inP = inputShape->at(0);

            std::vector<Nd4jLong> shape({shape::shapeOf(inP)[0], (Nd4jLong) block.getIArguments()->size()});
            auto newShape = ConstantShapeHelper::getInstance().createShapeInfo(ArrayOptions::dataType(inP), 'c', shape);
            return SHAPELIST(newShape);
        }

        DECLARE_TYPES(firas_sparse) {
            getOpDescriptor()
                    ->setAllowedInputTypes(sd::DataType::ANY)
                    ->setAllowedOutputTypes({ALL_FLOATS});
        }
    }
}

#endif

#endif //LIBND4J_THIRD_PARTY_H
