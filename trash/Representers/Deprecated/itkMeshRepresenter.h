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



#ifndef ITKMesh_REPRESENTER_H_
#define ITKMesh_REPRESENTER_H_

#include "statismo/Representer.h"
#include "itkMesh.h"
#include "itkObject.h"
#include "itkMesh.h"
#include "statismo_ITK/statismoITKConfig.h"
#include "statismo/CommonTypes.h"
#include <boost/unordered_map.hpp>

namespace statismo {
template <>
struct RepresenterTraits<itk::Mesh<float, 3u> > {

    typedef itk::Mesh<float, 3u> MeshType;

	typedef MeshType::Pointer DatasetPointerType;
	typedef MeshType::Pointer DatasetConstPointerType;

	typedef typename MeshType::PointType PointType;
	typedef typename MeshType::PointType ValueType;

	static void DeleteDataset(DatasetPointerType d) {};
    ///@}

};

}

namespace itk {

// helper function to compute the hash value of an itk point (needed by unorderd_map)
template <typename PointType>
int hash_value(const PointType& pt) {
	int hash_val = 1;
	for (unsigned i = 0; i < pt.GetPointDimension(); i++)
		hash_val *= pt[i];
	return hash_val;
}



/**
 * \ingroup Representers
 * \brief A representer for scalar valued itk Meshs
 * \sa Representer
 */
template <class TPixel, unsigned MeshDimension>
class MeshRepresenter : public statismo::Representer<itk::Mesh<TPixel, MeshDimension> >, public Object {
public:

	/* Standard class typedefs. */
	typedef MeshRepresenter            Self;
	typedef Object	Superclass;
	typedef SmartPointer<Self>                Pointer;
	typedef SmartPointer<const Self>          ConstPointer;

    typedef itk::Mesh<TPixel, MeshDimension> MeshType;
	typedef typename statismo::Representer<MeshType > RepresenterBaseType;
	typedef typename RepresenterBaseType::DomainType DomainType;
	typedef typename RepresenterBaseType::PointType PointType;
	typedef typename RepresenterBaseType::ValueType ValueType;
	typedef typename RepresenterBaseType::DatasetPointerType DatasetPointerType;
	typedef typename RepresenterBaseType::DatasetConstPointerType DatasetConstPointerType;

	/** New macro for creation of through a Smart Pointer. */
	itkSimpleNewMacro( Self );

	/** Run-time type information (and related methods). */
	itkTypeMacro( MeshRepresenter, Object );

	void Load(const H5::CommonFG& fg);
	MeshRepresenter* Clone() const;


	/// The type of the data set to be used
	typedef MeshType DatasetType;

	// Const correcness is difficult to enforce using smart pointers, as no conversion
	// between nonconst and const pointer are possible. Thus we define both to be non-const
	typedef typename MeshType::PointsContainer PointsContainerType;

	// An unordered map is used to cache pointid for corresonding points
	typedef boost::unordered_map<PointType, unsigned> PointCacheType;

	 // not used for this representer, but needs to be here as it is part of the generic interface
	struct DatasetInfo {};

	MeshRepresenter();
	virtual ~MeshRepresenter();

	unsigned GetDimensions() const { return MeshDimension; }

	std::string GetName() const { return "itkMeshRepresenter"; }


	const DomainType& GetDomain() const { return m_domain; }

	/** Set the reference that is used to build the model */
	void SetReference(DatasetConstPointerType ds);

	statismo::VectorType PointToVector(const PointType& pt) const;

	/**
	 * Create a sample from the dataset. No alignment or registration is done
	 */
	DatasetPointerType DatasetToSample(DatasetConstPointerType ds) const;

	/**
	 * Converts a sample to its vectorial representation
	 */
	statismo::VectorType SampleToSampleVector(DatasetConstPointerType sample) const;

	/**
	 * Converts the given sample Vector to a Sample (an itk::Mesh)
	 */
	DatasetPointerType SampleVectorToSample(const statismo::VectorType& sample) const;

	/**
	 * Returns the value of the sample at the point with the given id.
	 */
	ValueType PointSampleFromSample(DatasetConstPointerType sample, unsigned ptid) const;

	/**
	 * Given a vector, represening a points convert it to an itkPoint
	 */
	ValueType PointSampleVectorToPointSample(const statismo::VectorType& pointSample) const;

	/**
	 * Given an itkPoint, convert it to a sample vector
	 */
	statismo::VectorType PointSampleToPointSampleVector(const ValueType& v) const;

	/**
	 * Save the state of the representer (this simply saves the reference)
	 */
	void Save(const H5::CommonFG& fg) const;

	/// return the number of points of the reference
	virtual unsigned GetNumberOfPoints() const;

	/// return the point id associated with the given point
	/// \warning This works currently only for points that are defined on the reference
	virtual unsigned GetPointIdForPoint(const PointType& point) const;


    /// return the reference used in the representer
    DatasetConstPointerType GetReference() const { return m_reference; }

    void Delete() const {
    	this->UnRegister();
    	// the delete of itk is non-const. We call it from the const version
    	//const_cast<MeshRepresenter<TPixel, MeshDimension>*>(this) ->Delete();
    }

private:
    DatasetPointerType cloneMesh(const MeshType* mesh) const;

    static DatasetPointerType ReadDataset(const char* filename);

    static void WriteDataset(const char* filename, const MeshType* Mesh);


    // returns the closest point for the given mesh
    unsigned FindClosestPoint(const MeshType* mesh, const PointType pt) const ;

	DatasetConstPointerType m_reference;
	DomainType m_domain;
	mutable PointCacheType m_pointCache;
};

} // namespace itk

#include "itkMeshRepresenter.txx"

#endif /* itkMeshREPRESENTER_H_ */
