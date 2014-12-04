#ifndef __itkStandardImageRepresenterTraits_h
#define __itkStandardImageRepresenterTraits_h

#include <itkImage.h>
#include <itkVector.h>
#include "statismo/core/Representer.h"

namespace statismo {

template<>
struct RepresenterTraits<itk::Image<itk::Vector<float, 4u>, 4u> > {

    typedef itk::Image<itk::Vector<float, 4u>, 4u> VectorImageType;
    typedef VectorImageType::Pointer DatasetPointerType;
    typedef VectorImageType::Pointer DatasetConstPointerType;
    typedef VectorImageType::PointType PointType;
    typedef VectorImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<itk::Vector<float, 3u>, 3u> > {

    typedef itk::Image<itk::Vector<float, 3u>, 3u> VectorImageType;
    typedef VectorImageType::Pointer DatasetPointerType;
    typedef VectorImageType::Pointer DatasetConstPointerType;
    typedef VectorImageType::PointType PointType;
    typedef VectorImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<itk::Vector<float, 2u>, 2u> > {

    typedef itk::Image<itk::Vector<float, 2u>, 2u> VectorImageType;
    typedef VectorImageType::Pointer DatasetPointerType;
    typedef VectorImageType::Pointer DatasetConstPointerType;
    typedef VectorImageType::PointType PointType;
    typedef VectorImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<float, 4u> > {

    typedef itk::Image<float, 4u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<float, 3u> > {

    typedef itk::Image<float, 3u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<float, 2u> > {

    typedef itk::Image<float, 2u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<short, 4u> > {

    typedef itk::Image<short, 4u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<short, 3u> > {

    typedef itk::Image<short, 3u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<short, 2u> > {

    typedef itk::Image<float, 2u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<unsigned char, 4u> > {

    typedef itk::Image<char, 4u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<unsigned char, 3u> > {

    typedef itk::Image<char, 3u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};

template<>
struct RepresenterTraits<itk::Image<unsigned char, 2u> > {

    typedef itk::Image<short, 2u> ImageType;
    typedef ImageType::Pointer DatasetPointerType;
    typedef ImageType::Pointer DatasetConstPointerType;
    typedef ImageType::PointType PointType;
    typedef ImageType::PixelType ValueType;
};


} // namespace statismo

#endif
