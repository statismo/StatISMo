/*
 * Representer.txx
 *
 * Created by Remi Blanc, Marcel Luethi
 *
 * Copyright (c) 2011 ETH Zurich
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

#include "CommonTypes.h"
#include "Exceptions.h"
#include <iostream>

#include <Eigen/SVD>
#include "PCAModelBuilder.h"

namespace statismo {

//
// ConditionalModelBuilder
//
//


template <typename Representer>
unsigned
ConditionalModelBuilder<Representer>::PrepareData(const SampleDataStructureListType& sampleDataList,
                                                  const SurrogateTypeVectorType& surrogateTypes,
												  const CondVariableValueVectorType& conditioningInfo,
												  SampleDataStructureListType *acceptedSamples,
												  MatrixType *surrogateMatrix,
												  VectorType *conditions) const
{
	bool acceptSample;
	unsigned nbAcceptedSamples = 0;
	unsigned nbContinuousSurrogatesInUse = 0, nbCategoricalSurrogatesInUse = 0;
	std::vector<unsigned> indicesContinuousSurrogatesInUse;
	std::vector<unsigned> indicesCategoricalSurrogatesInUse;

	//first: identify the continuous and categorical variables, which are used for conditioning and which are not
	for (unsigned i=0 ; i<conditioningInfo.size() ; i++) {
		if (conditioningInfo[i].first) { //only variables that are used for conditioning are of interest here
			if (surrogateTypes[i] == SampleDataStructureWithSurrogatesType::Continuous) {
				nbContinuousSurrogatesInUse++;
				indicesContinuousSurrogatesInUse.push_back(i);
			}
			else {
				nbCategoricalSurrogatesInUse++;
				indicesCategoricalSurrogatesInUse.push_back(i);
			}
		}
	}
	conditions->resize(nbContinuousSurrogatesInUse);
	for (unsigned i=0 ; i<nbContinuousSurrogatesInUse ; i++) (*conditions)(i) = conditioningInfo[i].second;
	surrogateMatrix->resize(nbContinuousSurrogatesInUse, sampleDataList.size()); //number of variables is now known: nbContinuousSurrogatesInUse ; the number of samples is yet unknown... 
	
	//now, browse all samples to select the ones which fall into the requested categories
	for (typename SampleDataStructureListType::const_iterator it = sampleDataList.begin(); it != sampleDataList.end(); ++it)
	{
		const SampleDataStructureWithSurrogatesType* sampleData = dynamic_cast<const SampleDataStructureWithSurrogatesType*>(*it);
		if (sampleData == 0)  {
			// this is a normal sample without surrogate information.
			// we simply discard it
			continue;
		}

		VectorType surrogateData = sampleData->GetSurrogateVector();
		acceptSample = true;
		for (unsigned i=0 ; i<nbCategoricalSurrogatesInUse ; i++) { //check that this sample respect the requested categories
			if ( conditioningInfo[indicesCategoricalSurrogatesInUse[i]].second !=
				 surrogateData[indicesCategoricalSurrogatesInUse[i]] )
			{
				//if one of the categories does not fit the requested one, then the sample is discarded
				acceptSample = false;
				continue;
			}
		}

		if (acceptSample) { //if the sample is of the right category
			acceptedSamples->push_back(*it);
			//and fill in the matrix of continuous variables
			for (unsigned j=0 ; j<nbContinuousSurrogatesInUse ; j++) {
				(*surrogateMatrix)(j,nbAcceptedSamples) = surrogateData[indicesContinuousSurrogatesInUse[j]];
			}
			nbAcceptedSamples++;
		}
	}
	//resize the matrix of surrogate data to the effective number of accepted samples
	surrogateMatrix->conservativeResize(Eigen::NoChange_t(), nbAcceptedSamples);

	return nbAcceptedSamples;
}

template <typename Representer>
typename ConditionalModelBuilder<Representer>::StatisticalModelType*
ConditionalModelBuilder<Representer>::BuildNewModel(const SampleDataStructureListType& sampleDataList,
													const SurrogateTypeVectorType& surrogateTypes,
													const CondVariableValueVectorType& conditioningInfo,
													float noiseVariance) const
{

	SampleDataStructureListType acceptedSamples;
	MatrixType X;
	VectorType x0;
	unsigned nSamples = PrepareData(sampleDataList, surrogateTypes, conditioningInfo, &acceptedSamples, &X, &x0);
	assert(nSamples == acceptedSamples.size());

	unsigned nCondVariables = X.rows();

	// build a normal PCA model
	typedef PCAModelBuilder<Representer> PCAModelBuilderType;
	PCAModelBuilderType* modelBuilder = PCAModelBuilderType::Create();
	StatisticalModelType* pcaModel = modelBuilder->BuildNewModel(acceptedSamples, noiseVariance);
	unsigned nPCAComponents = pcaModel->GetNumberOfPrincipalComponents();
	
	if ( X.cols() == 0 || X.rows() == 0)
	{
		return pcaModel;
	}
	else
	{
		// the scores in the pca model correspond to the parameters of each sample in the model.
		MatrixType B = pcaModel->GetModelInfo().GetScoresMatrix().transpose();
		assert(B.rows() == nSamples);
		assert(B.cols() == nPCAComponents);

		// A is the joint data matrix B, X, where X contains the conditional information for each sample
		// Thus the i-th row of A contains the PCA parameters b of the i-th sample,
		// together with the conditional information for each sample
		MatrixType A(nSamples, nPCAComponents+nCondVariables);
		A << B,X.transpose();

		// Compute the mean and the covariance of the joint data matrix
		VectorType mu = A.colwise().mean().transpose(); // colwise returns a row vector
		assert(mu.rows() == nPCAComponents + nCondVariables);

		MatrixType A0 = A.rowwise() - mu.transpose(); //
		MatrixType cov = 1.0 / (nSamples-1) * A0.transpose() *  A0;

		assert(cov.rows() == cov.cols());
		assert(cov.rows() == pcaModel->GetNumberOfPrincipalComponents() + nCondVariables);

		// extract the submatrices involving the conditionals x
		// note that since the matrix is symmetrix, Sbx = Sxb.transpose(), hence we only store one
		MatrixType Sbx = cov.topRightCorner(nPCAComponents, nCondVariables);
		MatrixType Sxx = cov.bottomRightCorner(nCondVariables, nCondVariables);
		MatrixType Sbb = cov.topLeftCorner(nPCAComponents, nPCAComponents);

		// compute the conditional mean
		VectorType condMean = mu.topRows(nPCAComponents) + Sbx * Sxx.inverse() * (x0 - mu.bottomRows(nCondVariables));

		// compute the conditional covariance
		MatrixType condCov = Sbb - Sbx * Sxx.inverse() * Sbx.transpose();
		
		// get the sample mean corresponding the the conditional given mean of the parameter vectors
		VectorType condMeanSample = pcaModel->GetRepresenter()->SampleToSampleVector(pcaModel->DrawSample(condMean));


		// so far all the computation have been done in parameter (latent) space. Go back to sample space.
		// (see PartiallyFixedModelBuilder for a detailed documentation)
		// TODO we should factor this out into the base class, as it is the same code as it is used in
		// the partially fixed model builder
		const VectorType& pcaVariance = pcaModel->GetPCAVarianceVector();
		VectorTypeDoublePrecision pcaSdev = pcaVariance.cast<double>().array().sqrt();

		typedef Eigen::JacobiSVD<MatrixTypeDoublePrecision> SVDType;
		MatrixTypeDoublePrecision innerMatrix = pcaSdev.asDiagonal() * condCov.cast<double>() * pcaSdev.asDiagonal();
		SVDType svd(innerMatrix, Eigen::ComputeThinU);

		VectorType newPCAVariance = svd.singularValues().cast<ScalarType>();
		MatrixType newPCABasisMatrix = pcaModel->GetOrthonormalPCABasisMatrix() * svd.matrixU().cast<ScalarType>();

		StatisticalModelType* model = StatisticalModelType::Create(pcaModel->GetRepresenter(), condMeanSample, newPCABasisMatrix, newPCAVariance, noiseVariance);

		// add builder info and data info to the info list
		MatrixType scores(0,0);
		typename ModelInfo::BuilderInfoList bi;
		bi.push_back(ModelInfo::KeyValuePair("BuilderName ", "ConditionalModelBuilder"));
		bi.push_back(ModelInfo::KeyValuePair("NoiseVariance ", Utils::toString(noiseVariance)));
		
		//generate a matrix ; first column = boolean (yes/no, this variable is used) ; second: conditioning value.
		MatrixType conditioningInfoMatrix(conditioningInfo.size(), 2);
		for (unsigned i=0 ; i<conditioningInfo.size() ; i++) {
			conditioningInfoMatrix(i,0) = conditioningInfo[i].first;
			conditioningInfoMatrix(i,1) = conditioningInfo[i].second;
		}
		bi.push_back(ModelInfo::KeyValuePair("ConditioningInfo ", Utils::toString(conditioningInfoMatrix)));
		//Is this all that is necessary? do something special when loading?
		//bi.push_back(ModelInfo::KeyValuePair("WARNING ", "The conditional model builder does not save all of its parameters yet"));

		typename ModelInfo::DataInfoList di = pcaModel->GetModelInfo().GetDataInfo();

		ModelInfo info(scores, di, bi);
		model->SetModelInfo(info);

		delete pcaModel;

		return model;
	}

}


} // namespace statismo
