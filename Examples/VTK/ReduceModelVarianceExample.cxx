/*
 * This file is part of the statismo library.
 *
 * Author: Marcel Luethi (marcel.luethi@unibas.ch)
 *
 * Copyright (c) 2011 University of Basel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "statismo/StatisticalModel.h"
#include "statismo/ReducedVarianceModelBuilder.h"
#include "Representers/VTK/vtkPolyDataRepresenter.h"

#include "vtkPolyData.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"

#include <iostream>
#include <memory>

using namespace statismo;
using std::auto_ptr;


// illustrates how to reduce the variance of a model
int main(int argc, char** argv) {

	if (argc < 3) {
		std::cout << "Usage " << argv[0] << " inputmodel outputmodel" << std::endl;
		exit(-1);
	}
	std::string inputModelName(argv[1]);
	std::string outputModelName(argv[2]);


	// All the statismo classes have to be parameterized with the RepresenterType.
	// For building a shape model with vtk, we use the vtkPolyDataRepresenter.
	typedef vtkPolyDataRepresenter RepresenterType;
	typedef StatisticalModel<RepresenterType> StatisticalModelType;
	typedef ReducedVarianceModelBuilder<RepresenterType> ReducedVarianceModelBuilderType;

	try {

		// To load a model, we call the static Load method, which returns (a pointer to) a
		// new StatisticalModel object
		auto_ptr<StatisticalModelType> model(StatisticalModelType::Load(inputModelName));
		std::cout << "loaded model with variance of " << model->GetPCAVarianceVector().sum()  << std::endl;

		auto_ptr<ReducedVarianceModelBuilderType> reducedVarModelBuilder(ReducedVarianceModelBuilderType::Create());

		// build a model with only half the variance
		auto_ptr<StatisticalModelType> reducedModel(reducedVarModelBuilder->BuildNewModelFromModel(model.get(), 0.5));
		std::cout << "new model has variance of " << reducedModel->GetPCAVarianceVector().sum()  << std::endl;

		reducedModel->Save(outputModelName);
	}
	catch (StatisticalModelException& e) {
		std::cout << "Exception occured while building the shape model" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

