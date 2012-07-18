/*
   This is the main program of ICCDEF.
*/

#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include "itkImage.h"
#include "itkIndex.h"
#include "itkSize.h"
#include "itkExceptionObject.h"
#include "itkBrains2MaskImageIOFactory.h"
#include "ICCDEFWarp.h"
#include "iccdefRegistration_NewCLP.h"

#include "itkPDEDeformableRegistrationFilter.h"
#include "itkArray.h"
#include "itkIO.h"
#include "itkICCDeformableRegistrationFilter.h"
#include "ReadMask.h"
#include "itkLabelStatisticsImageFilter.h"

#include "commandIterationupdate.h"
#include "itkSpatialObjectReader.h"
#include "itkBrains2LandmarkReader.h"
#ifdef USE_DEBUG_IMAGE_VIEWER
#include "DebugImageViewerClient.h"
#endif

struct ICCDEFWarpAppParameters
  {
  std::string movingVolume;
  std::string fixedVolume;
  std::string outputDirectory;
  std::string outputPrefix;
  std::string inputPixelType;
  std::string outputPixelType;
  std::string outputForwardDeformationFieldVolume;
  std::string outputBackwardDeformationFieldVolume;

  bool forceCoronalZeroOrigin;
  bool outputDebug;

  int backgroundFillValue;
  itk::Size<3> medianFilterSize;
//  std::string initialDeformationFieldVolume;
  std::string initializeWithTransform;
  std::string initialFixedDeformationFieldVolume;
  std::string initialMovingDeformationFieldVolume;

  /** Smoothing sigma for the deformation field at each iteration.*/
  float smoothDeformationFieldSigma;

  /** Maximum lengthof an update vector. */
  float maxStepLength;

  std::string maskProcessingMode;
  std::string fixedBinaryVolume;
  std::string movingBinaryVolume;

  /** Landmark file name **/
  std::string fixedLandmark;
  std::string movingLandmark;
  // int lowerThresholdForBOBF;
  // int upperThresholdForBOBF;
  // itk::Index<3> seedForBOBF;
  // itk::Index<3> neighborhoodForBOBF;

  /** IterationArray type. */
  typedef itk::Array<unsigned int> IterationsArrayType;
  unsigned long numberOfHistogramLevels;
  unsigned long numberOfMatchPoints;
  unsigned short numberOfLevels;
  IterationsArrayType numberOfIterations;
  float regularizationWeight;
  float inverseWeight;
  float similarityWeight;
  float landmarkWeight;
  float min_Jacobian_value;
  float harmonic_percent;
  bool normalization;
  bool outputDisplacement;
  bool outputDeformationField;
  bool outputJacobianImage;
  bool useConsistentLandmark;
  bool useConsistentIntensity;
  };

// This function prints the valid pixel types.
void PrintDataTypeStrings(void)
{
  // Prints the Input and output data type strings.
  std::cout << "uchar" << std::endl;
  std::cout << "short" << std::endl;
  std::cout << "ushort" << std::endl;
  std::cout << "int" << std::endl;
  std::cout << "float" << std::endl;

#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes
  // too many segments in one object file for the intel compiler
  std::cout << "uint" << std::endl;
  std::cout << "double" << std::endl;
#endif
}

// This function compares strings ignoring case.
int CompareNoCase( const std::string & s, const std::string & s2 )
{
  // Compare strings.
  std::string::const_iterator p = s.begin();
  std::string::const_iterator p2 = s2.begin();

  while( p != s.end() && p2 != s2.end() )
    {
    if( toupper(*p) != toupper(*p2) )
      {
      return ( toupper(*p) < toupper(*p2) ) ? -1 : 1;
      }
    p++;
    p2++;
    }

  return ( s2.size() == s.size() ) ? 0 : ( s.size() < s2.size() ) ? -1 : 1;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template <class InPixelType, class OutPixelType>
void ThirionFunction(const struct ICCDEFWarpAppParameters & command)
{
  const int dims = 3;

  typedef itk::Image<InPixelType, dims>              ImageType;
  typedef itk::Image<float, dims>                    TRealImage;
  typedef itk::Image<OutPixelType, dims>             OutputImageType;
  typedef itk::Image<itk::Vector<float, dims>, dims> TDeformationField;
  // If optional landmark files given, will use landmark registration to
  // generate
  // a deformation field to prime the thirion demons registration.
  // Need to explicitly register the B2MaskIOFactory

  if( command.outputDirectory.size() != 0 )
    {
    if( itksys::SystemTools::FileIsDirectory(command.outputDirectory.c_str() ) )
      {
      itksys::SystemTools::ChangeDirectory(command.outputDirectory.c_str() );
      }
    else
      {
      std::cout << "Directory is not existed!" << std::endl;
      exit(-1);
      }
    }
  std::cout << "Working directory:" << itksys::SystemTools::GetCurrentWorkingDirectory() << std::endl;
  std::string workingPath = itksys::SystemTools::GetCurrentWorkingDirectory();
  if( itksys::SystemTools::FileIsDirectory(command.outputPrefix.c_str() ) )
    {
    std::string m_ForwardDir = command.outputPrefix + "/" + "forward";
    if( !itksys::SystemTools::FileIsDirectory(m_ForwardDir.c_str() ) )
      {
      itksys::SystemTools::MakeDirectory(m_ForwardDir.c_str() );
      }
    std::string m_BackwardDir = command.outputPrefix + "/" + "backward";
    if( !itksys::SystemTools::FileIsDirectory(m_BackwardDir.c_str() ) )
      {
      itksys::SystemTools::MakeDirectory(m_BackwardDir.c_str() );
      }
    }
  else
    {
    itksys::SystemTools::MakeDirectory(command.outputPrefix.c_str() );
    std::string m_ForwardDir = command.outputPrefix + "/" + "forward";
    itksys::SystemTools::MakeDirectory(m_ForwardDir.c_str() );

    std::string m_BackwardDir = command.outputPrefix + "/" + "backward";
    itksys::SystemTools::MakeDirectory(m_BackwardDir.c_str() );
    }

  // Create directory
//  itksys::SystemTools::MakeDirectory(command.outputPrefix.c_str());
//  std::string m_ForwardDir = command.outputPrefix + "/" + "forward";
//  itksys::SystemTools::MakeDirectory(m_ForwardDir.c_str());

//  std::string m_BackwardDir = command.outputPrefix + "/" + "backward";
//  itksys::SystemTools::MakeDirectory(m_BackwardDir.c_str());

  itksys::SystemTools::ChangeDirectory(command.outputPrefix.c_str() );

  typedef itk::SpatialObject<dims>        ImageMaskType;
  typedef typename ImageMaskType::Pointer ImageMaskPointer;

  ImageMaskPointer fixedMask = NULL;
  ImageMaskPointer movingMask = NULL;

  typedef typename itk::ICCDEFWarp
    <ImageType,
     TRealImage,
     OutputImageType
    > AppType;
  typename  AppType::Pointer app = AppType::New();

  // Set up the diffeomorphic demons filter with mask

  // Set up the demons filter

  typedef typename itk::ICCDeformableRegistrationFilter<TRealImage, TRealImage,
                                                        TDeformationField> ActualRegistrationFilterType;
  ActualRegistrationFilterType::Pointer filter
    = ActualRegistrationFilterType::New();

  filter->SetRegularizationWeight(command.regularizationWeight);
  filter->SetInverseWeight(command.inverseWeight);
  filter->SetMaximumUpdateStepLength(command.maxStepLength );
  filter->SetHarmonicPercent(command.harmonic_percent);
  filter->SetMinJac(command.min_Jacobian_value);

  //  filter->SmoothDeformationFieldOn();

  if( command.useConsistentIntensity )
    {
    filter->SetSimilarityWeight(command.similarityWeight);
    filter->SetUseConsistentIntensity(true);
    }

  if( command.useConsistentLandmark )
    {
    std::cout << "Landmark Matching....." << std::endl;
    filter->SetUseConsistentLandmark(true);
    filter->SetLandmarkWeight(command.landmarkWeight);
    if( command.fixedLandmark == "" || command.movingLandmark == "" )
      {
      std::cout
        << "ERROR:  Must specify landmark file names when usingConsistentLandmark is set to true" << std::endl;
      exit(-1);
      }
    else
      {
      // Read Landmark spatial objects
      // what type is the landmark file?
#if 0
      typedef  itk::SpatialObjectReader<dims, InPixelType> SpatialObjectReaderType;
      typedef  itk::LandmarkSpatialObject<dims>            LandmarkSpatialObjectType;
      typename SpatialObjectReaderType::Pointer fixedLandmarkReader = SpatialObjectReaderType::New();
      fixedLandmarkReader->SetFileName(command.fixedLandmark.c_str() );
      fixedLandmarkReader->Update();
      filter->SetFixedLandmark(dynamic_cast<LandmarkSpatialObjectType *>(fixedLandmarkReader->GetGroup().GetPointer() ) );

      typename SpatialObjectReaderType::Pointer movingLandmarkReader = SpatialObjectReaderType::New();
      movingLandmarkReader->SetFileName(command.movingLandmark.c_str() );
      movingLandmarkReader->Update();
      filter->SetMovingLandmark(
        dynamic_cast<LandmarkSpatialObjectType *>(movingLandmarkReader->GetGroup().GetPointer() ) );
#endif

      // Transform index point to physical point
      typename  TRealImage::Pointer fixedVolume
        = itkUtil::ReadImage<TRealImage>( command.fixedVolume.c_str() );
      fixedVolume = itkUtil::OrientImage<TRealImage>(fixedVolume,
                                                     itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
      typename  TRealImage::Pointer movingVolume
        = itkUtil::ReadImage<TRealImage>( command.movingVolume.c_str() );
      movingVolume = itkUtil::OrientImage<TRealImage>(movingVolume,
                                                      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);

      typedef itk::Brains2LandmarkReader<InPixelType, dims> Brains2LandmarkReaderType;
      typename Brains2LandmarkReaderType::Pointer fixedLandmarkReader = Brains2LandmarkReaderType::New();
      fixedLandmarkReader->SetFileName(command.fixedLandmark.c_str() );
      fixedLandmarkReader->SetReferenceImage(fixedVolume);
      fixedLandmarkReader->Update();
      filter->SetFixedLandmark(fixedLandmarkReader->GetPhysicalPointSet() );

      typename Brains2LandmarkReaderType::Pointer movingLandmarkReader = Brains2LandmarkReaderType::New();
      movingLandmarkReader->SetFileName(command.movingLandmark.c_str() );
      movingLandmarkReader->SetReferenceImage(movingVolume);
      movingLandmarkReader->Update();
      filter->SetMovingLandmark(movingLandmarkReader->GetPhysicalPointSet() );
      }
    }
  if( !(filter->GetUseConsistentIntensity() ) && !(filter->GetUseConsistentLandmark() ) )
    {
    std::cout
      << "ERROR:  Must specify either Consistent intensity or Consistent landmark!" << std::endl;
    exit(-1);
    }

#if 0
  if( command.maskProcessingMode == "ROI" )
    {
    if( ( command.fixedBinaryVolume == "" )
        || ( command.movingBinaryVolume == "" ) )
      {
      std::cout
        << "ERROR:  Must specify mask file names when ROI is used for the maskProcessingMode" << std::endl;
      exit(-1);
      }
    std::cout << "Registration with Mask!!!!!!!" << std::endl;
    typename  TRealImage::Pointer fixedVolume
      = itkUtil::ReadImageRAI<TRealImage>( command.fixedVolume.c_str() );
    typename  TRealImage::Pointer movingVolume
      = itkUtil::ReadImageRAI<TRealImage>( command.movingVolume.c_str() );

    fixedMask = ReadImageMask<ImageMaskType, dims>(
        command.fixedBinaryVolume,
        fixedVolume->GetDirection(),
        fixedVolume->GetOrigin() );
    movingMask = ReadImageMask<ImageMaskType, dims>(
        command.movingBinaryVolume,
        movingVolume->GetDirection(),
        movingVolume->GetOrigin() );

    filter->SetFixedImageMask(fixedMask);
    filter->SetMovingImageMask(movingMask);
    filter->SetBackgroundFilledValue(command.backgroundFillValue);
    }

#endif

  typename CommandIterationUpdate<float,  3>::Pointer observer
    = CommandIterationUpdate<float, 3>::New();

  if( command.outputDebug )
    {
    std::ofstream fid(command.outputPrefix.c_str() );
    filter->AddObserver( itk::IterationEvent(), observer );
    }

  app->SetRegistrationFilter(filter);

#if 0
  if( command.initialDeformationFieldVolume != "" )
    {
    app->SetInitialDeformationFieldFilename(
      command.initialDeformationFieldVolume.c_str() );
    }
#endif

  if( command.initialFixedDeformationFieldVolume != "" )
    {
    app->SetInitialFixedDeformationFieldFilename(
      command.initialFixedDeformationFieldVolume.c_str() );
    }

  if( command.initialMovingDeformationFieldVolume != "" )
    {
    app->SetInitialMovingDeformationFieldFilename(
      command.initialMovingDeformationFieldVolume.c_str() );
    }

  if( command.initializeWithTransform != "" )
    {
    app->SetInitialTransformFilename( command.initializeWithTransform.c_str() );
    }

  app->SetTheMovingImageFilename( command.movingVolume.c_str() );
  app->SetTheFixedImageFilename( command.fixedVolume.c_str() );
  if( command.outputDebug )
    {
    typename  TRealImage::Pointer fixedVolume
      = itkUtil::ReadImage<TRealImage>( command.fixedVolume.c_str() );
    typename  TRealImage::Pointer movingVolume
      = itkUtil::ReadImage<TRealImage>( command.movingVolume.c_str() );
    observer->SetMovingImage(movingVolume);
    observer->SetFixedImage(fixedVolume);
    }

//  app->SetWarpedImageName ( command.outputVolume.c_str () );
  app->SetOutputPrefix(command.outputPrefix);

  app->SetMedianFilterSize(command.medianFilterSize);

  if( command.outputForwardDeformationFieldVolume != "" )
    {
    app->SetForwardDeformationFieldOutputName(command.outputForwardDeformationFieldVolume);
    }
  if( command.outputBackwardDeformationFieldVolume != "" )
    {
    app->SetBackwardDeformationFieldOutputName(command.outputBackwardDeformationFieldVolume);
    }

  // Set the other optional arguments if specified by the user.
  if( command.outputDisplacement )
    {
    app->SetOutputDisplacement( command.outputDisplacement );
    }
  if( command.outputDeformationField )
    {
    app->SetOutputDeformationField(command.outputDeformationField);
    }
  if( command.outputJacobianImage )
    {
    app->SetOutputJacobianImage(command.outputJacobianImage);
    }

  app->SetForceCoronalZeroOrigin(command.forceCoronalZeroOrigin);
  if( command.outputDebug )
    {
    bool debug = true;
    app->SetOutDebug(debug);    // TODO:  SetOutDebug should be a boolean not a
                                // string.
    }

  app->SetUseHistogramMatching(command.normalization);
  if( app->GetUseHistogramMatching() )
    {
    if( command.outputDebug )
      {
      std::cout << " Use Histogram Matching....." << std::endl;
      }
    app->SetNumberOfHistogramLevels(command.numberOfHistogramLevels);
    app->SetNumberOfMatchPoints(command.numberOfMatchPoints);
    }

  app->SetNumberOfLevels(command.numberOfLevels);
  app->SetNumberOfIterations(command.numberOfIterations);
  // If making BOBF option is specified Initialize its parameters

  if( command.maskProcessingMode == "BOBF" )
    {
    if( ( command.fixedBinaryVolume == "" )
        || ( command.movingBinaryVolume == "" ) )
      {
      std::cout
        <<
        "Error: If BOBF option is set for maskProcessingMode then the fixed mask name and moving mask file name should be specified. \n";
      exit(-1);
      }

    app->SetFixedBinaryVolume( command.fixedBinaryVolume.c_str() );
    app->SetMovingBinaryVolume( command.movingBinaryVolume.c_str() );
    if( command.outputDebug )
      {
      std::cout << "Setting Default PixelValue: "
                << command.backgroundFillValue << "." << std::endl;
      }
    app->SetDefaultPixelValue(command.backgroundFillValue);
    }
  if( command.outputDebug )
    {
    std::cout << "Running ICCDEF Registration...." << std::endl;
    }
  try
    {
    app->Execute();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << "Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch( ... )
    {
    std::cout << "Caught a non-ITK exception " << __FILE__ << " " << __LINE__
              << std::endl;
    }

  return;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template <class InPixelType, class OutPixelType>
void ProcessAppType(const struct ICCDEFWarpAppParameters & command)
{
  ThirionFunction<InPixelType, OutPixelType>(command);
}

// This function processes the output data type.
template <class PixelType>
void
ProcessOutputType(struct ICCDEFWarpAppParameters & command)
{
  if( command.outputPixelType != "" )
    {
    // process the string for the data type
    if( CompareNoCase( command.outputPixelType, std::string("uchar") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned char>(command);
      }
    else if( CompareNoCase( command.outputPixelType,
                            std::string("short") ) == 0 )
      {
      ProcessAppType<PixelType, short>(command);
      }
    else if( CompareNoCase( command.outputPixelType,
                            std::string("ushort") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned short>(command);
      }
    else if( CompareNoCase( command.outputPixelType,
                            std::string("int") ) == 0 )
      {
      ProcessAppType<PixelType, int>(command);
      }
    else if( CompareNoCase( command.outputPixelType,
                            std::string("float") ) == 0 )
      {
      ProcessAppType<PixelType, float>(command);
      }
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
    else if( CompareNoCase( command.outputPixelType,
                            std::string("uint") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned int>(command);
      }
    else if( CompareNoCase( command.outputPixelType,
                            std::string("double") ) == 0 )
      {
      ProcessAppType<PixelType, double>(command);
      }
#endif
    else
      {
      std::cout
        << "Error. Invalid data type for -outtype!  Use one of these:"
        << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }
  else
    {
    ProcessAppType<PixelType, float>(command);
    }
}

#ifdef USE_DEBUG_IMAGE_VIEWER
/*************************
 * Have a global variable to
 * add debugging information.
 */
DebugImageViewerClient DebugImageDisplaySender;
#endif

int ICCDEFWarpPrimary(int argc, char *argv[])
{
  struct ICCDEFWarpAppParameters command;

    {
    PARSE_ARGS;
    std::cout << "Viewer:" << UseDebugImageViewer << std::endl;
#ifdef USE_DEBUG_IMAGE_VIEWER
    DebugImageDisplaySender.SetEnabled(UseDebugImageViewer);
    DebugImageDisplaySender.SetPromptUser(PromptAfterImageSend);
#endif

    command.movingVolume = movingVolume;
    command.fixedVolume = fixedVolume;
    command.outputPrefix = outputPrefix;
    command.outputDirectory = outputDirectory;
    command.outputForwardDeformationFieldVolume = outputForwardDeformationFieldVolume;
    command.outputBackwardDeformationFieldVolume = outputBackwardDeformationFieldVolume;
    command.inputPixelType = inputPixelType;
    command.outputPixelType = outputPixelType;
//    command.outputDisplacementFieldPrefix = outputDisplacementFieldPrefix;
    command.forceCoronalZeroOrigin = forceCoronalZeroOrigin;

    command.outputDebug = outputDebug;

    command.backgroundFillValue = backgroundFillValue;

//    command.initialDeformationFieldVolume = initialDeformationFieldVolume;
    command.initialFixedDeformationFieldVolume = initialFixedDeformationFieldVolume;
    command.initialMovingDeformationFieldVolume = initialMovingDeformationFieldVolume;
//	command.jacobianImagePrefix = jacobianImagePrefix;
    command.initializeWithTransform = initializeWithTransform;
    command.regularizationWeight = regularizationWeight;
    command.similarityWeight = similarityWeight;
    command.inverseWeight = inverseWeight;
    command.landmarkWeight = landmarkWeight;
    command.min_Jacobian_value = min_Jacobian_value;
    command.harmonic_percent = harmonic_percent;
    command.useConsistentLandmark = useConsistentLandmark;
    command.useConsistentIntensity = useConsistentIntensity;

    command.numberOfHistogramLevels = numberOfHistogramBins;
    command.numberOfMatchPoints = numberOfMatchPoints;
    command.numberOfLevels = numberOfPyramidLevels;
    command.numberOfIterations.SetSize(numberOfPyramidLevels);

    command.maxStepLength = maxStepLength;
    command.smoothDeformationFieldSigma = smoothDeformationFieldSigma;
    command.normalization = normalization;
    command.outputDisplacement = outputDisplacement;
    command.outputDeformationField = outputDeformationField;
    command.outputJacobianImage = outputJacobianImage;

    command.maskProcessingMode = maskProcessingMode;
    command.fixedBinaryVolume = fixedBinaryVolume;
    command.movingBinaryVolume = movingBinaryVolume;
    command.fixedLandmark = fixedLandmark;
    command.movingLandmark = movingLandmark;
//    command.lowerThresholdForBOBF = lowerThresholdForBOBF;
//    command.upperThresholdForBOBF = upperThresholdForBOBF;
    for( int i = 0; i < numberOfPyramidLevels; i++ )
      {
      command.numberOfIterations[i] = arrayOfPyramidLevelIterations[i];
      }
    for( int i = 0; i < 3; i++ )
      {
      command.medianFilterSize[i] = medianFilterSize[i];
//      command.seedForBOBF[i] = seedForBOBF[i];
//      command.neighborhoodForBOBF[i] = neighborhoodForBOBF[i];
      }

    if( outputPixelType == "CopyInputPixelType" )
      {
      command.outputPixelType = inputPixelType;
      }
    }

  //  bool debug=true;
  if( command.outputDebug )
    {
    std::cout
      << "                        movingVolume: " << command.movingVolume
      << std::endl
      << "                         fixedVolume: " << command.fixedVolume
      << std::endl
      << "                     outputDirectory: " << command.outputDirectory
      << std::endl
      << "              useConsistentIntensity: " << command.useConsistentIntensity
      << std::endl
      << "               useConsistentLandmark: " << command.useConsistentLandmark
      << std::endl
      << " outputForwardDeformationFieldVolume: " << command.outputForwardDeformationFieldVolume
      << std::endl
      << "outputBackwardDeformationFieldVolume: " << command.outputBackwardDeformationFieldVolume
      << std::endl
      << "                        outputPrefix: " << command.outputPrefix
      << std::endl
      << "                      inputPixelType: " << command.inputPixelType
      << std::endl
      << "                     outputPixelType: " << command.outputPixelType
      << std::endl
      << "                         outputDebug: " << command.outputDebug
      << std::endl
      << "                 backgroundFillValue: " << command.backgroundFillValue
      << std::endl
      << "                    medianFilterSize: " << command.medianFilterSize
      << std::endl
      << "  initialFixedDeformationFieldVolume: " << command.initialFixedDeformationFieldVolume
      << std::endl
      << " initialMovingDeformationFieldVolume: " << command.initialMovingDeformationFieldVolume
      << std::endl
      << "             initializeWithTransform: " << command.initializeWithTransform
      << std::endl
      << "                   fixedBinaryVolume: " << command.fixedBinaryVolume
      << std::endl
      << "                  movingBinaryVolume: " << command.movingBinaryVolume
      << std::endl
      << "                       fixedLandmark: " << command.fixedLandmark
      << std::endl
      << "                      movingLandmark: " << command.movingLandmark
      << std::endl
      << "                       maxStepLength: " << command.maxStepLength
      << std::endl
      << "         smoothDeformationFieldSigma: " << command.smoothDeformationFieldSigma
      << std::endl
      << "                    histogram levels: "
      << command.numberOfHistogramLevels << std::endl
      << "                     matching points: " << command.numberOfMatchPoints
      << std::endl
      << "                    harmonic_percent: " << command.harmonic_percent << std::endl
      << "              minimal Jacobian value: " << command.min_Jacobian_value << std::endl
      << "                   similarity Weight: " << command.similarityWeight << std::endl
      << "                      Inverse Weight: " << command.inverseWeight   << std::endl
      << "               Regularization Weight: " << command.regularizationWeight   << std::endl
      << "                     landmark Weight: " << command.landmarkWeight   << std::endl;
    ;
    }

  bool violated = false;
  if( command.movingVolume.size() == 0 )
    {
    violated = true; std::cout << "  --movingVolume Required! "  << std::endl;
    }
  if( command.fixedVolume.size() == 0 )
    {
    violated = true; std::cout << "  --fixedVolume Required! "  << std::endl;
    }

  // if (outputVolume.size() == 0) { violated = true; std::cout << "
  //  --outputVolume Required! "  << std::endl; }
  // if (outputDeformationFieldVolume.size() == 0) { violated = true; std::cout
  // << "  --outputDeformationFieldVolume Required! "  << std::endl; }
  // if (registrationParameters.size() == 0) { violated = true; std::cout << "
  //  --registrationParameters Required! "  << std::endl; }
  // if (inputPixelType.size() == 0) { violated = true; std::cout << "
  //  --inputPixelType Required! "  << std::endl; }
  if( violated )
    {
    exit(1);
    }

  // Test if the input data type is valid
  if( command.inputPixelType != "" )
    {
    // check to see if valid type
    if( ( CompareNoCase( command.inputPixelType.c_str(), std::string("uchar" ) ) )
        && ( CompareNoCase( command.inputPixelType.c_str(), std::string("short" ) ) )
        && ( CompareNoCase( command.inputPixelType.c_str(),
                            std::string("ushort") ) )
        && ( CompareNoCase( command.inputPixelType.c_str(), std::string("int"   ) ) )
        && ( CompareNoCase( command.inputPixelType.c_str(), std::string("float" ) ) )
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
        &&
        ( CompareNoCase( command.inputPixelType.c_str(), std::string("uint"  ) ) )
        && ( CompareNoCase( command.inputPixelType.c_str(),
                            std::string("double") ) )
#endif
        )
      {
      std::cout
        << "Error. Invalid data type string specified with --inputPixelType!"
        << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  if( command.outputPixelType != "" )
    {
    // check to see if valid type
    if( ( CompareNoCase( command.outputPixelType.c_str(),
                         std::string("uchar" ) ) )
        &&            ( CompareNoCase( command.outputPixelType.c_str(),
                                       std::string("SHORT") ) )
        && ( CompareNoCase( command.outputPixelType.c_str(),
                            std::string("ushort") ) )
        && ( CompareNoCase( command.outputPixelType.c_str(), std::string("int"   ) ) )
        && ( CompareNoCase( command.outputPixelType.c_str(),
                            std::string("float" ) ) )
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
        &&
        ( CompareNoCase( command.outputPixelType.c_str(), std::string("uint"  ) ) )
        && ( CompareNoCase( command.outputPixelType.c_str(),
                            std::string("double") ) )
#endif
        )
      {
      std::cout
        << "Error. Invalid data type string specified with --outputPixelType!"
        << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  // Call the process output data type function based on the input data type.

  if( CompareNoCase( command.inputPixelType, std::string("uchar") ) == 0 )
    {
    ProcessOutputType<unsigned char>(command);
    }
  else if( CompareNoCase( command.inputPixelType,
                          std::string("short") ) == 0 )
    {
    ProcessOutputType<short>(command);
    }
  else if( CompareNoCase( command.inputPixelType,
                          std::string("ushort") ) == 0 )
    {
    ProcessOutputType<unsigned short>(command);
    }
  else if( CompareNoCase( command.inputPixelType, std::string("int") ) == 0 )
    {
    ProcessOutputType<int>(command);
    }
  else if( CompareNoCase( command.inputPixelType,
                          std::string("float") ) == 0 )
    {
    ProcessOutputType<float>(command);
    }
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
  else if( CompareNoCase( command.inputPixelType, std::string("uint") ) == 0 )
    {
    ProcessOutputType<unsigned int>(command);
    }
  else if( CompareNoCase( command.inputPixelType,
                          std::string("double") ) == 0 )
    {
    ProcessOutputType<double>(command);
    }
#endif
  else
    {
    std::cout
      << "Error. Invalid data type for --inputPixelType!  Use one of these:"
      << std::endl;
    PrintDataTypeStrings();
    exit(-1);
    }
  return 0;
}
