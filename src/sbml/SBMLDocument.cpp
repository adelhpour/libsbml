/**
 * @file    SBMLDocument.cpp
 * @brief   Implementation of the top-level container for an SBML Model and
 *          associated data. 
 * @author  Ben Bornstein
 *
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright (C) 2020 jointly by the following organizations:
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. University of Heidelberg, Heidelberg, Germany
 *     3. University College London, London, UK
 *
 * Copyright (C) 2019 jointly by the following organizations:
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. University of Heidelberg, Heidelberg, Germany
 *
 * Copyright (C) 2013-2018 jointly by the following organizations:
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 *     3. University of Heidelberg, Heidelberg, Germany
 *
 * Copyright (C) 2009-2013 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 *  
 * Copyright (C) 2006-2008 by the California Institute of Technology,
 *     Pasadena, CA, USA 
 *  
 * Copyright (C) 2002-2005 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. Japan Science and Technology Agency, Japan
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution
 * and also available online as http://sbml.org/software/libsbml/license.html
 * ---------------------------------------------------------------------- -->*/

#include <iostream>

#include <sbml/xml/XMLAttributes.h>
#include <sbml/xml/XMLNamespaces.h>
#include <sbml/xml/XMLInputStream.h>
#include <sbml/xml/XMLOutputStream.h>
#include <sbml/xml/XMLLogOverride.h>
#include <sbml/xml/XMLError.h>

#include <sbml/validator/SBMLInternalValidator.h>
#include <sbml/validator/StrictUnitConsistencyValidator.h>
#include <sbml/validator/UnitConsistencyValidator.h>

#include <sbml/Model.h>
#include <sbml/SBMLErrorLog.h>
#include <sbml/SBMLVisitor.h>
#include <sbml/SBMLError.h>
#include <sbml/SBMLDocument.h>
#include <sbml/SBMLReader.h>
#include <sbml/SBMLWriter.h>

#include <sbml/extension/SBMLExtensionRegistry.h>
#include <sbml/extension/SBasePluginCreatorBase.h>
#include <sbml/extension/SBasePlugin.h>
#include <sbml/extension/SBMLDocumentPlugin.h>

#include <sbml/conversion/ConversionProperties.h>
#include <sbml/conversion/SBMLConverterRegistry.h>

#include <sbml/util/ElementFilter.h>

/** @cond doxygenIgnored */
using namespace std;
/** @endcond */

LIBSBML_CPP_NAMESPACE_BEGIN
#ifdef __cplusplus


/*
 * Get the most recent Level of SBML supported by this release of
 * libSBML.
 *
 * This is the "default" level in the sense that libSBML will create
 * models of this SBML Level unless told otherwise.
 * 
 * @return the number representing the most recent SBML specification level
 * (at the time this libSBML was released).
 */
unsigned int
SBMLDocument::getDefaultLevel ()
{
  return SBML_DEFAULT_LEVEL;
}


/*
 * Get the most recent Version with the most recent Level of SBML supported
 * by this release of libSBML.
 *
 * This is the "default" version in the sense that libSBML will create
 * models of this SBML Level and Version unless told otherwise.
 * 
 * @return the number representing the most recent SBML specification
 * version (at the time this libSBML was released).
 */
unsigned int
SBMLDocument::getDefaultVersion ()
{
  return SBML_DEFAULT_VERSION;
}


/*
 * Creates a new SBMLDocument.  If not specified, the SBML level and
 * version attributes default to the most recent SBML specification (at the
 * time this libSBML was released).
 */
SBMLDocument::SBMLDocument (unsigned int level, unsigned int version) :
   SBase (level, version)
 , mLevel   ( level   )
 , mVersion ( version )
 , mModel   ( NULL       )
 , mLocationURI     ("")
 , mRequiredAttrOfUnknownPkg()
 , mRequiredAttrOfUnknownDisabledPkg()
{
  if (mLevel   == 0 && mVersion == 0)  
  {
    mLevel   = getDefaultLevel  ();
    mVersion = getDefaultVersion();

    mSBMLNamespaces->setLevel(mLevel);
    mSBMLNamespaces->setVersion(mVersion);
    XMLNamespaces *ns = new XMLNamespaces();
    ns->add(SBMLNamespaces::getSBMLNamespaceURI(mLevel, mVersion));
    mSBMLNamespaces->setNamespaces(ns);
    delete ns;
  }

  if (!hasValidLevelVersionNamespaceCombination())
    throw SBMLConstructorException();

  mInternalValidator = new SBMLInternalValidator();
  mInternalValidator->setDocument(this);
  mInternalValidator->setApplicableValidators(AllChecksON);
  mInternalValidator->setConversionValidators(AllChecksON);

  mSBML = this;

  setElementNamespace(mSBMLNamespaces->getURI());
}


SBMLDocument::SBMLDocument (SBMLNamespaces* sbmlns) :
   SBase  (sbmlns)
 , mModel ( NULL       )
 , mLocationURI ("")
 , mRequiredAttrOfUnknownPkg()
 , mRequiredAttrOfUnknownDisabledPkg()
{
  if (!hasValidLevelVersionNamespaceCombination())
  {
    throw SBMLConstructorException(SBMLDocument::getElementName(), sbmlns);
  }

  mInternalValidator = new SBMLInternalValidator();
  mInternalValidator->setDocument(this);
  mInternalValidator->setApplicableValidators(AllChecksON);
  mInternalValidator->setConversionValidators(AllChecksON);

  mSBML = this;
  mLevel   = sbmlns->getLevel();
  mVersion = sbmlns->getVersion();

  //
  // (TODO) Namespace check for extension packages 
  //        would need to be improved
  //
  //
  // (EXTENSION)
  //

  loadPlugins(sbmlns);

  //
  // (TODO) Checks if objects in mPlugins are SBMLDocumentPlugin derived
  //        objects.
  //
}

/** @cond doxygenLibsbmlInternal */
  unsigned int SBMLDocument::getNumValidators() const
  {
    return (unsigned int)mValidators.size();
  }
  int SBMLDocument::clearValidators()
  {
    list<SBMLValidator*>::iterator it;
    for (it = mValidators.begin(); it != mValidators.end(); it++)
    {
      delete *it;
    }
    mValidators.clear();
    return LIBSBML_OPERATION_SUCCESS;
  }
  int SBMLDocument::addValidator(const SBMLValidator* validator)
  {
    mValidators.push_back(validator->clone());
    return LIBSBML_OPERATION_SUCCESS;
  }

  SBMLValidator* SBMLDocument::getValidator(unsigned int index)
  {
    if (index >= getNumValidators()) return NULL;
    list<SBMLValidator*>::iterator it;
    unsigned int count = 0;
    for (it = mValidators.begin(); it != mValidators.end(); it++)
    {
      if (count == index) return *it;
    }
    return NULL;
  }
/** @endcond */


/*
 * Destroys this SBMLDocument.
 */
SBMLDocument::~SBMLDocument ()
{
  if (mInternalValidator != NULL)
    delete mInternalValidator;
  if (mModel != NULL)
  {
    // remove from static map, since this object is being deleted
		SBMLTransforms::clearComponentValues(mModel);
    delete mModel;
  }
  clearValidators();
}


/*
 * Creates a copy of this SBMLDocument.
 */
SBMLDocument::SBMLDocument (const SBMLDocument& orig)
 : SBase  ( orig          )
 , mLevel ( orig.mLevel   )
 , mVersion ( orig.mVersion )
 , mModel ( NULL          )
 , mLocationURI (orig.mLocationURI )
 , mErrorLog()
 , mValidators ()
 , mInternalValidator(new SBMLInternalValidator())
 , mRequiredAttrOfUnknownPkg(orig.mRequiredAttrOfUnknownPkg)
 , mRequiredAttrOfUnknownDisabledPkg(orig.mRequiredAttrOfUnknownDisabledPkg)
 , mPkgUseDefaultNSMap()
{
  
  
  SBMLDocument::setSBMLDocument(this);
  
  mInternalValidator->setDocument(this);
  mInternalValidator->setApplicableValidators(orig.getApplicableValidators());
  mInternalValidator->setConversionValidators(orig.getConversionValidators());
  
  if (orig.mModel != NULL) 
  {
    mModel = static_cast<Model*>( orig.mModel->clone() );
    mModel->setSBMLDocument(this);
  }
  
  SBMLDocument::connectToChild();
}


/*
 * Assignment operator of this SBMLDocument.
 */
SBMLDocument& SBMLDocument::operator=(const SBMLDocument& rhs)
{
  if(&rhs!=this)
  {
    this->SBase::operator =(rhs);
    setSBMLDocument(this);

    mLevel                             = rhs.mLevel;
    mVersion                           = rhs.mVersion;
    mLocationURI                       = rhs.mLocationURI;

    if (mInternalValidator != NULL)
    {
      delete mInternalValidator;
    }
    mInternalValidator = (SBMLInternalValidator*)rhs.mInternalValidator->clone();
    mInternalValidator->setDocument(this);
    mRequiredAttrOfUnknownPkg = rhs.mRequiredAttrOfUnknownPkg;
    mRequiredAttrOfUnknownDisabledPkg = rhs.mRequiredAttrOfUnknownDisabledPkg;

    if (rhs.mModel != NULL) 
    {
      mModel = rhs.mModel->clone();
      mModel->setSBMLDocument(this);
    }
  }
  connectToChild();
  return *this;

}


/** @cond doxygenLibsbmlInternal */
bool
SBMLDocument::accept (SBMLVisitor& v) const
{
  v.visit(*this);
  if (mModel != NULL) mModel->accept(v);
  v.leave(*this);

  return true;
}
/** @endcond */


/*
 * @return a (deep) copy of this SBMLDocument.
 */
SBMLDocument*
SBMLDocument::clone () const
{
  return new SBMLDocument(*this);
}


bool 
SBMLDocument::isSetModel() const
{
  return mModel != NULL;
}


/*
 * @return the Model contained in this SBMLDocument.
 */
const Model*
SBMLDocument::getModel () const
{
  return mModel;
}


/*
 * @return the Model contained in this SBMLDocument.
 */
Model*
SBMLDocument::getModel ()
{
  return mModel;
}

/** @cond doxygenLibsbmlInternal */

/*
 * Returns the number of "elementName" in this Reaction.
 */
unsigned int
SBMLDocument::getNumObjects(const std::string& elementName)
{
  unsigned int n = 0;

  if (elementName == "model" && isSetModel())
    return 1;

  return n;
}

/** @endcond */



/** @cond doxygenLibsbmlInternal */

/*
 * Returns the nth object of "objectName" in this Reaction.
 */
SBase*
SBMLDocument::getObject(const std::string& elementName, unsigned int index)
{
  SBase* obj = NULL;

  if (elementName == "model")
  {
    return getModel();
  }

  return obj;
}

/** @endcond */

SBase* 
SBMLDocument::getElementBySId(const std::string& id)
{
  if (id.empty()) return NULL;
  if (mModel != NULL) {
    if (mModel->getId() == id) return mModel;
    SBase* obj = mModel->getElementBySId(id);
    if (obj != NULL) return obj;
  }
  return getElementFromPluginsBySId(id);
}


SBase*
SBMLDocument::getElementByMetaId(const std::string& metaid)
{
  if (metaid.empty()) return NULL;
  if (getMetaId()==metaid) return this;
  if (mModel != NULL) {
    if (mModel->getMetaId() == metaid) return mModel;
    SBase * obj = mModel->getElementByMetaId(metaid);
    if (obj != NULL) return obj;
  }
  return getElementFromPluginsByMetaId(metaid);
}

List*
SBMLDocument::getAllElements(ElementFilter *filter)
{
  List* ret = new List();
  List* sublist = NULL;
  
  ADD_FILTERED_POINTER(ret, sublist, mModel, filter);  
  
  ADD_FILTERED_FROM_PLUGIN(ret, sublist, filter);

  return ret;
}


/** @cond doxygenLibsbmlInternal */
unsigned char
SBMLDocument::getApplicableValidators() const
{
  return mInternalValidator->getApplicableValidators();
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
unsigned char
SBMLDocument::getConversionValidators() const
{
  return mInternalValidator->getConversionValidators();
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
void
SBMLDocument::setApplicableValidators(unsigned char appl)
{
  return mInternalValidator->setApplicableValidators(appl);
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
void
SBMLDocument::setConversionValidators(unsigned char appl)
{
  return mInternalValidator->setConversionValidators(appl);
}
/** @endcond */


/* 
 * removes FD and expands them in math elements
 */
bool
SBMLDocument::expandFunctionDefinitions()
{
  ConversionProperties prop(getSBMLNamespaces());
  prop.addOption("expandFunctionDefinitions", true, "expand function definitions");

  if (convert(prop) == LIBSBML_OPERATION_SUCCESS)
    return true;
  else
    return false;
}


bool
SBMLDocument::expandInitialAssignments()
{
  
  ConversionProperties prop(getSBMLNamespaces());
  prop.addOption("expandInitialAssignments", true, "expand initial assignments");

  if (convert(prop) == LIBSBML_OPERATION_SUCCESS)
    return true;
  else
    return false;
}


/*
 * Sets the level and version of this SBMLDocument.  Valid
 * combinations are currently:
 *
 * @li Level 1 Version 2
 * @li Level 2 Version 1
 * @li Level 2 Version 2
 * @li Level 2 Version 3
 *
 * @note Some models cannot be converted from their existing
 * level and version to other particular combinations.
 * This function checks whether the required conversion 
 * is possible.
 */
bool
SBMLDocument::setLevelAndVersion (unsigned int level, unsigned int version,
                                  bool strict, bool ignorePackages)
{
  SBMLNamespaces sbmlns(level, version);
  ConversionProperties prop(&sbmlns);
  prop.addOption("strict", strict, "should validity be preserved");
  prop.addOption("setLevelAndVersion", true, "convert the document to the given level and version");
  prop.addOption("ignorePackages", ignorePackages);

  if (convert(prop) == LIBSBML_OPERATION_SUCCESS)
    return true;
  else
    return false;
}

/** @cond doxygenLibsbmlInternal */

/* 
 * function to set level to 0 on a doc that was just been created to read in to
 * the SBMLReader will only do this if the file is found to be invalid
 * this will allow for testing for an SBMLDocument without
 * relying on it having a model to be valid 
 * (in L3V2 a missing model will be valid) 
 */
void 
SBMLDocument::setInvalidLevel()
{
  mLevel = 0;
  mVersion = 0;
}

/** @endcond */

/** @cond doxygenLibsbmlInternal */
void 
SBMLDocument::updateSBMLNamespace(const std::string& package, unsigned int level,
                            unsigned int version)
{
  SBase::updateSBMLNamespace(package, level, version);
  if (package.empty() || package == "core")
  {
    mLevel = level;
    mVersion = version;
  }

  if (isSetModel())
  {
    mModel->updateSBMLNamespace(package, level, version);
  }
}
/** @endcond */


/*
 * Sets the Model for this SBMLDocument to a copy of the given Model.
 */
int
SBMLDocument::setModel (const Model* m)
{
  int returnValue = checkCompatibility(static_cast<const SBase *>(m));
  
  if (returnValue == LIBSBML_OPERATION_FAILED && m == NULL)
  {
    delete mModel;
    mModel = NULL;
    return LIBSBML_OPERATION_SUCCESS;
  }
  else if (returnValue != LIBSBML_OPERATION_SUCCESS)
  {
    return returnValue;
  }
  
  if (mModel == m)
  {
    return LIBSBML_OPERATION_SUCCESS;
  }
  else
  {
    delete mModel;
    mModel = (m != NULL) ? new Model(*m) : NULL;

    if (mModel != NULL) 
    {
      mModel->connectToParent(this);
    }

    if (mModel != NULL && getURI() != mModel->getURI()) 
    {
      mModel->setElementNamespace(getURI());
    }
    
    return LIBSBML_OPERATION_SUCCESS;
  }
}


/*
 * Creates a new Model (optionally with its id attribute set) inside this
 * SBMLDocument and returns it.
 */
Model*
SBMLDocument::createModel (const std::string sid)
{
  if (mModel != NULL) delete mModel;
  mModel = NULL;

  try
  {
    mModel = new Model(getSBMLNamespaces());
  }
  catch (SBMLConstructorException &)
  {
    /* here we do not create a default object as the level/version must
     * match the parent object
     *
     * so do nothing
     */
    return NULL;
  }
  
  if (mModel != NULL)
  {
    mModel->setId(sid);

    mModel->connectToParent(this);
  }
  return mModel;
}


void 
SBMLDocument::setLocationURI (const std::string& uri)
{
  mLocationURI = uri;
}


std::string 
SBMLDocument::getLocationURI() const
{
  return mLocationURI;
}

std::string 
SBMLDocument::getLocationURI()
{
  return mLocationURI;
}


void 
SBMLDocument::setConsistencyChecks(SBMLErrorCategory_t category,
                                   bool apply)
{
  return mInternalValidator->setConsistencyChecks(category, apply);
}


void 
SBMLDocument::setConsistencyChecksForConversion(SBMLErrorCategory_t category,
                                   bool apply)
{
  return mInternalValidator->setConsistencyChecksForConversion(category, apply);
}


/*
 * Performs a set of semantic consistency checks on the document.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkConsistency ()
{
  // keep a copy of the override status
  // and then override any change
  XMLErrorSeverityOverride_t overrideStatus = 
                                  getErrorLog()->getSeverityOverride();
  getErrorLog()->setSeverityOverride(LIBSBML_OVERRIDE_DISABLED);

  unsigned int numErrors = mInternalValidator->checkConsistency();

  for (unsigned int i = 0; i < getNumPlugins(); i++)
  {
    numErrors += static_cast<SBMLDocumentPlugin*>
                      (getPlugin(i))->checkConsistency();
  }

  list<SBMLValidator*>::iterator it;
  for (it = mValidators.begin(); it != mValidators.end(); it++)
  {
    long newErrors = (*it)->validate(*this);
    if (newErrors > 0)
    {
      mErrorLog.add((*it)->getFailures());
      numErrors += newErrors;
    }
  }

  // restore value of override
  getErrorLog()->setSeverityOverride(overrideStatus);

  return numErrors;
}


/*
 * Performs a set of semantic consistency checks on the document.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkConsistencyWithStrictUnits ()
{
  // keep a copy of the override status
  // and then override any change
  XMLErrorSeverityOverride_t overrideStatus = 
                                  getErrorLog()->getSeverityOverride();
  getErrorLog()->setSeverityOverride(LIBSBML_OVERRIDE_DISABLED);

  // turn off the original units validator
  setConsistencyChecks(LIBSBML_CAT_UNITS_CONSISTENCY, false);

  unsigned int numErrors = mInternalValidator->checkConsistency();

  for (unsigned int i = 0; i < getNumPlugins(); i++)
  {
    numErrors += static_cast<SBMLDocumentPlugin*>
                      (getPlugin(i))->checkConsistency();
  }

  list<SBMLValidator*>::iterator it;
  for (it = mValidators.begin(); it != mValidators.end(); it++)
  {
    long newErrors = (*it)->validate(*this);
    if (newErrors > 0)
    {
      mErrorLog.add((*it)->getFailures());
      numErrors += newErrors;
    }
  }

  // check we have no serious errors 
  bool seriousErrors = getNumErrors(LIBSBML_SEV_FATAL) > 0
    || getNumErrors(LIBSBML_SEV_ERROR) > 0;

  if (seriousErrors)
  {
    // restore value of override
    getErrorLog()->setSeverityOverride(overrideStatus);

    return numErrors;
  }
  else
  {
    // log as errors
    getErrorLog()->setSeverityOverride(LIBSBML_OVERRIDE_ERROR);
    StrictUnitConsistencyValidator unit_validator;
    unit_validator.init();
    unsigned int nerrors = unit_validator.validate(*this);
    numErrors += nerrors;
    if (nerrors > 0) 
    {
      getErrorLog()->add( unit_validator.getFailures() );
    }
  }
    


  // restore value of override
  getErrorLog()->setSeverityOverride(overrideStatus);

  return numErrors;
}


/*
 * Performs consistency checking and validation on this SBML document.
 *
 * If this method returns a nonzero value (meaning, one or more
 * consistency checks have failed for SBML document), the failures may be
 * due to warnings @em or errors.  Callers should inspect the severity
 * flag in the individual SBMLError objects returned by
 * SBMLDocument::getError(@if java long n@endif) to determine the nature of the failures.
 *
 * @note unlike checkConsistency this method will write the document
 *       in order to determine all errors for the document. This will 
 *       also clear the error log. 
 *
 * @return the number of failed checks (errors) encountered.
 *
 * @see SBMLDocument::checkConsistency()
 */
unsigned int SBMLDocument::validateSBML ()
{
  // keep a copy of the override status
  // and then override any change
  XMLErrorSeverityOverride_t overrideStatus = 
                                  getErrorLog()->getSeverityOverride();
  getErrorLog()->setSeverityOverride(LIBSBML_OVERRIDE_DISABLED);

  unsigned int numErrors = mInternalValidator->checkConsistency();

  for (unsigned int i = 0; i < getNumPlugins(); i++)
  {
    numErrors += static_cast<SBMLDocumentPlugin*>
      (getPlugin(i))->checkConsistency();
  }

  list<SBMLValidator*>::iterator it;
  for (it = mValidators.begin(); it != mValidators.end(); it++)
  {
    long newErrors = (*it)->validate(*this);
    if (newErrors > 0)
    {
      mErrorLog.add((*it)->getFailures());
      numErrors += newErrors;
    }
  }
  // restore value of override
  getErrorLog()->setSeverityOverride(overrideStatus);

  return numErrors;
}


/*
 * Performs consistency checking on libSBML's internal representation of 
 * an SBML Model.
 *
 * Callers should query the results of the consistency check by calling
 * getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkInternalConsistency()
{
  // keep a copy of the override status
  // and then override any change
  XMLErrorSeverityOverride_t overrideStatus = 
                                  getErrorLog()->getSeverityOverride();
  getErrorLog()->setSeverityOverride(LIBSBML_OVERRIDE_DISABLED);

  unsigned int numErrors = mInternalValidator->checkInternalConsistency();

  // restore value of override
  getErrorLog()->setSeverityOverride(overrideStatus);

  return numErrors;
}

unsigned int
getLevelVersionSeverity(unsigned int errorId, unsigned int level, unsigned int version)
{
  return SBMLError(errorId, level, version).getSeverity();
}

/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L1 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL1Compatibility (bool inConversion)
{
  unsigned int nerrors =  mInternalValidator->checkL1Compatibility();
  unsigned int unit_errors = 0;

  if (inConversion == false)
  {
    UnitConsistencyValidator unit_validator;
    unit_validator.init();
    unit_errors = unit_validator.validate(*this);
    if (unit_errors > 0)
    {
      // need to check whether these are what would be warnings
      // and only log the error if there would be a units error
      bool logUnitError = false;
      std::list<SBMLError> errors = unit_validator.getFailures();
      std::list<SBMLError>::iterator it = errors.begin();

      while(!logUnitError && it != errors.end())
      {
        SBMLError err = (SBMLError)*it;
        unsigned int l2v1_sev = getLevelVersionSeverity(err.getErrorId(), 1, 2);
        logUnitError = (l2v1_sev == LIBSBML_SEV_ERROR);
        it++;
      }
      if (logUnitError)
      {
        getErrorLog()->logError(StrictUnitsRequiredInL1, getLevel(), getVersion());
        unit_errors = 1;
      }
      else
      {
        unit_errors = 0;
      }
    }
  }

  return nerrors + unit_errors;
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L2v1 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL2v1Compatibility (bool inConversion)
{
  unsigned int nerrors = mInternalValidator->checkL2v1Compatibility();
  unsigned int unit_errors = 0;

  if (inConversion == false)
  {
    UnitConsistencyValidator unit_validator;
    unit_validator.init();
    unit_errors = unit_validator.validate(*this);
    if (unit_errors > 0)
    {
      // need to check whether these are what would be warnings
      // and only log the error if there would be a units error
      bool logUnitError = false;
      std::list<SBMLError> errors = unit_validator.getFailures();
      std::list<SBMLError>::iterator it = errors.begin();

      while(!logUnitError && it != errors.end())
      {
        SBMLError err = (SBMLError)*it;
        unsigned int l2v1_sev = getLevelVersionSeverity(err.getErrorId(), 2, 1);
        if (l2v1_sev == LIBSBML_SEV_ERROR)
        {
          logUnitError = true;
        }
        it++;
      }
      if (logUnitError)
      {
        getErrorLog()->logError(StrictUnitsRequiredInL2v1, getLevel(), getVersion());
        unit_errors = 1;
      }
      else
      {
        unit_errors = 0;
      }
    }
  }

  return nerrors + unit_errors;
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L2v2 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL2v2Compatibility (bool inConversion)
{
  unsigned int nerrors =  mInternalValidator->checkL2v2Compatibility();
  unsigned int unit_errors = 0;

  if (inConversion == false)
  {
    UnitConsistencyValidator unit_validator;
    unit_validator.init();
    unit_errors = unit_validator.validate(*this);
    if (unit_errors > 0)
    {
      // need to check whether these are what would be warnings
      // and only log the error if there would be a units error
      bool logUnitError = false;
      std::list<SBMLError> errors = unit_validator.getFailures();
      std::list<SBMLError>::iterator it = errors.begin();

      while(!logUnitError && it != errors.end())
      {
        SBMLError err = (SBMLError)*it;
        unsigned int l2v1_sev = getLevelVersionSeverity(err.getErrorId(), 1, 2);
        logUnitError = (l2v1_sev == LIBSBML_SEV_ERROR);
        it++;
      }
      if (logUnitError)
      {
        getErrorLog()->logError(StrictUnitsRequiredInL2v2, getLevel(), getVersion());
        unit_errors = 1;
      }
      else
      {
        unit_errors = 0;
      }
    }
  }

  return nerrors + unit_errors;
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L2v3 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL2v3Compatibility (bool inConversion)
{
  unsigned int nerrors =  mInternalValidator->checkL2v3Compatibility();
  unsigned int unit_errors = 0;

  if (inConversion == false)
  {
    UnitConsistencyValidator unit_validator;
    unit_validator.init();
    unit_errors = unit_validator.validate(*this);
    if (unit_errors > 0)
    {
      // need to check whether these are what would be warnings
      // and only log the error if there would be a units error
      bool logUnitError = false;
      std::list<SBMLError> errors = unit_validator.getFailures();
      std::list<SBMLError>::iterator it = errors.begin();

      while(!logUnitError && it != errors.end())
      {
        SBMLError err = (SBMLError)*it;
        unsigned int l2v1_sev = getLevelVersionSeverity(err.getErrorId(), 1, 2);
        logUnitError = (l2v1_sev == LIBSBML_SEV_ERROR);
        it++;
      }
      if (logUnitError)
      {
        getErrorLog()->logError(StrictUnitsRequiredInL2v3, getLevel(), getVersion());
        unit_errors = 1;
      }
      else
      {
        unit_errors = 0;
      }
    }
  }

  return nerrors + unit_errors;
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L2v4 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL2v4Compatibility ()
{
  return mInternalValidator->checkL2v4Compatibility();
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L2v4 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL2v5Compatibility ()
{
  return mInternalValidator->checkL2v5Compatibility();
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L3v1 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL3v1Compatibility ()
{
  return mInternalValidator->checkL3v1Compatibility();
}


/*
 * Performs a set of semantic consistency checks on the document to establish
 * whether it is compatible with L3v2 and can be converted.  Query
 * the results by calling getNumErrors() and getError().
 *
 * @return the number of failed checks (errors) encountered.
 */
unsigned int
SBMLDocument::checkL3v2Compatibility()
{
  return mInternalValidator->checkL3v2Compatibility();
}


/*
 * @return the nth error encountered during the parse of this
 * SBMLDocument or @c NULL if n > getNumErrors() - 1.
 */
const SBMLError*
SBMLDocument::getError (unsigned int n) const
{
  return mErrorLog.getError(n);
}

const SBMLError*
SBMLDocument::getErrorWithSeverity(unsigned int n, unsigned int severity) const
{
  return mErrorLog.getErrorWithSeverity(n, severity);
}


/*
 * @return the number of errors encountered during the parse of this
 * SBMLDocument.
 */
unsigned int
SBMLDocument::getNumErrors () const
{
  return mErrorLog.getNumErrors();
}


unsigned int 
SBMLDocument::getNumErrors (unsigned int severity) const
{
  return ((getErrorLog() != NULL) ? 
      getErrorLog()->getNumFailsWithSeverity(severity) : 0);
}


/*
 * Prints all errors encountered during the parse of this SBMLDocument to
 * the given stream.  If no errors have occurred, i.e.  getNumErrors() ==
 * 0, no output will be sent to stream. The format of the output is:
 *
 *   N error(s):
 *     line N: (id) message
 */
void
SBMLDocument::printErrors (std::ostream& stream) const
{
  getErrorLog()->printErrors(stream);
}

void
SBMLDocument::printErrors(std::ostream& stream, unsigned int severity) const
{
  getErrorLog()->printErrors(stream, severity);
}


/** @cond doxygenLibsbmlInternal */
/*
 * Sets the parent SBMLDocument of this SBML object.
 */
void
SBMLDocument::setSBMLDocument (SBMLDocument* d)
{
  SBase::setSBMLDocument(d);
  // No-op
}

/*
 * Sets this SBML object to child SBML objects (if any).
 * (Creates a child-parent relationship by the parent)
  */
void
SBMLDocument::connectToChild()
{
  SBase::connectToChild();
  if (mModel) mModel->connectToParent(this);
  connectToParent(this);
}



int SBMLDocument::convert(const ConversionProperties& props)
{
  SBMLConverter* converter = SBMLConverterRegistry::getInstance().getConverterFor(props);

  if (converter == NULL) return LIBSBML_CONV_CONVERSION_NOT_AVAILABLE;

  converter->setDocument(this);
  converter->setProperties(&props);
  int result = converter->convert();

  delete converter;

  return result;
}
/** @endcond */


/*
 * @return the typecode (int) of this SBML object or SBML_UNKNOWN
 * (default).
 *
 * @see getElementName()
 */
int
SBMLDocument::getTypeCode () const
{
  return SBML_DOCUMENT;
}


/*
 * @return the name of this element ie "sbml".
 */
const string&
SBMLDocument::getElementName () const
{
  static const string name = "sbml";
  return name;
}


/** @cond doxygenLibsbmlInternal */
/*
 * @return the ordinal position of the element with respect to its siblings
 * or -1 (default) to indicate the position is not significant.
 */
int
SBMLDocument::getElementPosition () const
{
  return 1;
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
/*
 * @return the SBML object corresponding to next XMLToken in the
 * XMLInputStream or @c NULL if the token was not recognized.
 */
SBase*
SBMLDocument::createObject (XMLInputStream& stream)
{
  const string& name   = stream.peek().getName();
  SBase*        object = NULL;


  if (name == "model")
  {
    if (mModel != NULL)
    {
      if (getLevel() < 3 || (getLevel() == 3 && getVersion() < 2)) 
      {
        logError(NotSchemaConformant, getLevel(), getVersion(),
          "Only one <model> element is permitted inside a "
          "document.");
      }
      else
      {
        logError(MissingModel, getLevel(), getVersion());
      }
    }
    delete mModel;

    try
    {
      mModel = new Model(getSBMLNamespaces());
    }
    catch (SBMLConstructorException &)
    {
      mModel = new Model(SBMLDocument::getDefaultLevel(),
        SBMLDocument::getDefaultVersion());
    }

    object = mModel;
  }

  return object;
}
/** @endcond */


/*
  * @return the Namespaces associated with this SBML object
  */
XMLNamespaces* 
SBMLDocument::getNamespaces() const
{
  return mSBMLNamespaces->getNamespaces();
}


/*
 * @return the SBMLErrorLog used to log errors while reading and
 * validating SBML.
 */
SBMLErrorLog*
SBMLDocument::getErrorLog ()
{
  return &mErrorLog;
}


/*
 * @return the SBMLErrorLog used to log errors while reading and
 * validating SBML.
 */
const SBMLErrorLog*
SBMLDocument::getErrorLog () const
{
  return &mErrorLog;
}


int
SBMLDocument::enableDefaultNS(const std::string& package, bool flag)
{
  std::string pkgURI = "";
  for (size_t i=0; i < mPlugins.size(); i++)
  {
    std::string uri = mPlugins[i]->getURI();
    const SBMLExtension* sbext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);
    if (    (uri == package)
         || (sbext && (sbext->getName() == package))
       )	     
    {
      pkgURI = uri;
    }
  }

  if (pkgURI.empty()) 
  {
    return LIBSBML_PKG_UNKNOWN_VERSION;
  }

  PkgUseDefaultNSMapIter it = mPkgUseDefaultNSMap.find(pkgURI);
  if (it != mPkgUseDefaultNSMap.end()) 
  {
    (*it).second = flag;
  }
  else
  {
    mPkgUseDefaultNSMap.insert(pair<string,bool>(pkgURI,flag));
  }

  return LIBSBML_OPERATION_SUCCESS;
}


bool 
SBMLDocument::isEnabledDefaultNS(const std::string& package)
{
  std::string pkgURI;
  for (size_t i=0; i < mPlugins.size(); i++)
  {
    std::string uri = mPlugins[i]->getURI();
    const SBMLExtension* sbext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);
    if (    (uri == package)
         || (sbext && (sbext->getName() == package))
       )	     
    {
      pkgURI = uri;
    }
  }

  PkgUseDefaultNSMapIter it = mPkgUseDefaultNSMap.find(pkgURI);

  return (it != mPkgUseDefaultNSMap.end()) ? (*it).second : false;
}


int
SBMLDocument::setPackageRequired(const std::string& package, bool flag)
{
  for (size_t i=0; i < mPlugins.size(); i++)
  {
    std::string uri = mPlugins[i]->getURI();
    const SBMLExtension* sbext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);
    if (   (uri == package)
         || (sbext && (sbext->getName() == package))
       )
    {
      //
      // (NOTE) objects in mPlugins must be derived
      //        from SBMLDocumentPlugin to invoke the corresponding
      //        setReuired(), getRequired() functions
      //
      return static_cast<SBMLDocumentPlugin*>(mPlugins[i])->setRequired(flag);
    }
  }

  //
  // checks required attributes in unknown packages
  //
  if (mRequiredAttrOfUnknownPkg.getValue("required",package) != "")
  {
    int index = mRequiredAttrOfUnknownPkg.getIndex("required",package);
    std::string prefix = mRequiredAttrOfUnknownPkg.getPrefix(index);
    std::string value = (flag) ? "true" : "false";

    mRequiredAttrOfUnknownPkg.add("required", value, package, prefix);
    return LIBSBML_OPERATION_SUCCESS;
  }

  return LIBSBML_PKG_UNKNOWN_VERSION;
}


/** @cond doxygenLibsbmlInternal */
int
SBMLDocument::addUnknownPackageRequired(const std::string& pkgURI,
                                const std::string& prefix, bool flag)
{
  std::string value = (flag) ? "true" : "false";

  return mRequiredAttrOfUnknownPkg.add("required", value, pkgURI, prefix);
}
/** @endcond */


int
SBMLDocument::setPkgRequired(const std::string& package, bool flag)
{
  return setPackageRequired(package,flag);
}


bool 
SBMLDocument::getPackageRequired(const std::string& package)
{
  for (size_t i=0; i < mPlugins.size(); i++)
  {
    std::string uri = mPlugins[i]->getURI();
    const SBMLExtension* sbext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);
    if (   (uri == package)
        || (sbext && (sbext->getName() == package) )
       )
    {
      //
      // (NOTE) objects in mPlugins must be derived
      //        from SBMLDocumentPlugin to invoke the corresponding
      //        setRequired(), getRequired() functions
      //
      return static_cast<SBMLDocumentPlugin*>(mPlugins[i])->getRequired();
    }
  }

  //
  // checks required attributes in unknown packages
  //
  std::string req = mRequiredAttrOfUnknownPkg.getValue("required",package);
  if (req == "true") return true;

  return false;
}

bool 
SBMLDocument::getPkgRequired(const std::string& package)
{
  return getPackageRequired(package);
}

bool 
SBMLDocument::isSetPackageRequired(const std::string& package)
{
  for (size_t i=0; i < mPlugins.size(); i++)
  {
    std::string uri = mPlugins[i]->getURI();    
    const SBMLExtension* sbext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);

    if (   (uri == package)
        || (sbext && (sbext->getName() == package) )
       )
    {
      return true;
    }
  }

  //
  // checks required attributes in unknown packages
  //
  std::string req = mRequiredAttrOfUnknownPkg.getValue("required",package);
  if (!req.empty()) return true;

  return false;
}

bool 
SBMLDocument::isSetPkgRequired(const std::string& package)
{
  return isSetPackageRequired(package);
}

/*
 * Returnes @c true if the given package extension is one of ignored
 * packages (i.e. the package is defined in this document but the package
 * is not available), otherwise returns @c false.
 */
bool 
SBMLDocument::isIgnoredPackage(const std::string& pkgURI)
{
  if (isSetPackageRequired(pkgURI) && !isPackageURIEnabled(pkgURI))
    return true;

  return false;
}

/*
 * Returnes @c true if the given package extension is one of ignored
 * packages (i.e. the package is defined in this document but the package
 * is not available), otherwise returns @c false.
 */
bool 
SBMLDocument::isDisabledIgnoredPackage(const std::string& pkgURI) const
{
  if (!isPackageURIEnabled(pkgURI))
  {
    std::string req = 
            mRequiredAttrOfUnknownDisabledPkg.getValue("required", pkgURI);
    
    if (!req.empty()) 
    {
      return true;
    }
  }

  return false;
}


/** @cond doxygenLibsbmlInternal */
bool
SBMLDocument::hasUnknownPackage(const std::string& pkgURI) const
{
  // has this package been added to the list of unknown required attributes
  std::string req = mRequiredAttrOfUnknownPkg.getValue("required", pkgURI);
  return !req.empty();
}


int 
SBMLDocument::getNumUnknownPackages() const
{
  int count = 0;
  for (int i = 0; i < mRequiredAttrOfUnknownPkg.getLength(); ++i)
  {
    if (mRequiredAttrOfUnknownPkg.getName(i) == "required")
      ++count;
  }
  return count;
}

std::string 
SBMLDocument::getUnknownPackageURI(int index) const
{
  std::string result;
  for (int i = 0; i < mRequiredAttrOfUnknownPkg.getLength(); ++i)
  {
    if (mRequiredAttrOfUnknownPkg.getName(i) != "required")
      continue;
    if (i == index)
      return mRequiredAttrOfUnknownPkg.getURI(i);
  }
  return result;
}

std::string
SBMLDocument::getUnknownPackagePrefix(int index) const
{
  std::string result;
  for (int i = 0; i < mRequiredAttrOfUnknownPkg.getLength(); ++i)
  {
    if (mRequiredAttrOfUnknownPkg.getName(i) != "required")
      continue;
    if (index == i)
      return mRequiredAttrOfUnknownPkg.getPrefix(i);
  }
  return result;
}
/** @endcond */


bool 
SBMLDocument::isIgnoredPkg(const std::string& pkgURI)
{
  return isIgnoredPackage(pkgURI);
}

/** @cond doxygenLibsbmlInternal */
/*
 * Subclasses should override this method to get the list of
 * expected attributes.
 * This function is invoked from corresponding readAttributes()
 * function.
 */
void
SBMLDocument::addExpectedAttributes(ExpectedAttributes& attributes)
{
  SBase::addExpectedAttributes(attributes);

  //
  // (NOTICE)
  //
  // getLevel() and getVersion() functions MUST NOT BE USED in this
  // function, because level and version are unknown until the level 
  // and version attributes parsed by readAttributes() function.
  //

  attributes.add("level");
  attributes.add("version");
  attributes.add("schemaLocation");
}


/*
 * Subclasses should override this method to read values from the given
 * XMLAttributes set into their specific fields.  Be sure to call your
 * parent's implementation of this method as well.
 */
void
SBMLDocument::readAttributes (const XMLAttributes& attributes,
                              const ExpectedAttributes& expectedAttributes)
{
  //
  // level: positiveInteger  { use="required" fixed="1" }  (L1v1)
  // level: positiveInteger  { use="required" fixed="2" }  (L2v1)
  //
  bool levelRead = attributes.readInto("level", mLevel, getErrorLog(), false, getLine(), getColumn());

  //
  // version: positiveInteger  { use="required" fixed="1" }  (L1v1, L2v1)
  // version: positiveInteger  { use="required" fixed="2" }  (L1v2, L2v2)
  // version: positiveInteger  { use="required" fixed="3" }  (L2v3)
  //
  bool versionRead = attributes.readInto("version", mVersion, getErrorLog(), false, getLine(), getColumn());

  // 
  // (EXTENSION)  SBMLDocument specific code
  //
  // The code for creating an extension IF below is almost equal to that in 
  // SBase::SBase(SBMLNamespaces*,typecode (int)).
  //
  XMLNamespaces *xmlns = getNamespaces();
  if (xmlns == NULL)
  {
//     std::string err("SBase: xmlns is empty");
//     XMLNamespaces* xmlns = sbmlns->getNamespaces();
//     std::ostringstream oss;
//     oss << "\nTypeCode " << typeCode << endl;
//     if (xmlns)
//     {
//       XMLOutputStream xos(oss);
//       xos << *xmlns;
//     }
//     err.append(oss.str());
//     throw SBMLConstructorException(err);
  }
  else
  {
    int numxmlns= xmlns->getLength();
    for (int i=0; i < numxmlns; i++)
    {
      const std::string& uri = xmlns->getURI(i);
      const SBMLExtension* sbmlext = SBMLExtensionRegistry::getInstance().getExtensionInternal(uri);

      if (sbmlext && sbmlext->isEnabled())
      {
        // if we are in l3v2 and there exists an l3v2 version for the package
        // we wont accept the l3v1 version
        if (sbmlext->getVersion(uri) < 2 && this->getVersion() > 1)
        {
          std::string dummyURI;
          dummyURI.assign(uri);
          size_t pos = dummyURI.find("level3");
          if (pos != std::string::npos)
          {
            dummyURI.replace(pos, 15, "level3/version2");
            if (sbmlext->getVersion(dummyURI) == 2)
            {
              ostringstream msg;

              msg << "Package '" << xmlns->getPrefix(i) <<
                "' has a L3V2V1 specification which must be used in an L3V2 document.";
              logError(InvalidPackageLevelVersion, mLevel, mVersion, msg.str());
              return;


            }
          }
        }



        const std::string &prefix = xmlns->getPrefix(i);
        SBaseExtensionPoint extPoint(getPackageName(), SBML_DOCUMENT);
        const SBasePluginCreatorBase* sbPluginCreator = sbmlext->getSBasePluginCreator(extPoint);
        if (sbPluginCreator)
        {
          // (debug)
          //cout << "sbPlugin " << sbPlugin << endl;
          //sbPlugin->createPlugin(uri,prefix);
          // (debug)
          SBasePlugin* entity = sbPluginCreator->createPlugin(uri,prefix,xmlns);
          entity->connectToParent(this);
          mPlugins.push_back(entity);
        }
      }
      else
      {
        //
        //  1) Checks if there exists a "required" attribute with the prefix of
        //     this namespace.
        //  2) If such attribute exists then checks if the value is true or false.
        //  3) Logs an error (e..g The package is required but the package is not available)
        //     if the value is true.
        //  4) Added a check that the uri could possibly be a l3 ns
        //
        size_t pos = uri.find("http://www.sbml.org/sbml/level3/version");
        std::string requiredAttr = attributes.getValue("required",uri);
        if (pos == 0 && !requiredAttr.empty())
        {
          mRequiredAttrOfUnknownPkg.add("required", requiredAttr, uri, xmlns->getPrefix(i));
#if 0
          cout << "[DEBUG] SBMLDocument::readAttributes() uri " << uri 
               << " has required attribute : " << attributes.getValue("required",uri) << endl;
#endif
          ostringstream msg;

          if (requiredAttr == "true")
          {
            msg << "Package '" << xmlns->getPrefix(i) << 
                "' is a required package and the model cannot be properly "
                "interpreted.";
            logError(RequiredPackagePresent, mLevel, mVersion, msg.str());
          }
          else
          {
            msg << "Package '" << xmlns->getPrefix(i) << 
                "' is not a required package. The information relating "
                "to '" << xmlns->getPrefix(i) << "' will be "
                "saved but cannot be interpreted.";
            logError(UnrequiredPackagePresent, mLevel, mVersion, msg.str());
          }
        } 
      }
    }
  }

 
  //
  // (NOTE)
  //
  // In SBMLDocument, level and version are unknown until the attributes
  // read by the above function calls.
  //
  ExpectedAttributes addedEA(expectedAttributes);
 
  if (getLevel() > 2)
    addedEA.add("required");

  SBase::readAttributes(attributes,addedEA);


  /* check that the level and version are valid */
  if (mLevel == 1)
  {
    if (mVersion > 2)
    {
      logError(InvalidSBMLLevelVersion);
    }
  }
  else if (mLevel == 2)
  {
    if (mVersion > 5)
    {
      logError(InvalidSBMLLevelVersion);
    }
  }
  else if (mLevel == 3)
  {
    if (mVersion > 2)
    {
      logError(InvalidSBMLLevelVersion);
    }
  }
  else
  {
    logError(InvalidSBMLLevelVersion);
    return;
  }
  
  /* check that sbml namespace has been set */
  XMLNamespaces const *ns = mSBMLNamespaces->getNamespaces();
  unsigned int match = 0;
  if (ns == NULL)
  {
    logError(InvalidNamespaceOnSBML);
  }
  else 
  {
    for (int n = 0; n < ns->getLength(); n++)
    {
      if (!strcmp(ns->getURI(n).c_str(), 
                  "http://www.sbml.org/sbml/level1"))
      {
        match = 1;
        if (mLevel != 1 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if ((mVersion != 1 && mVersion != 2) || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
       break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level2"))
      {
        match = 1;
        if (mLevel != 2 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 1 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level2/version2"))
      {
        match = 1;
        if (mLevel != 2 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 2 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level2/version3"))
      {
        match = 1;
        if (mLevel != 2 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 3 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level2/version4"))
      {
        match = 1;
        if (mLevel != 2 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 4 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level2/version5"))
      {
        match = 1;
        if (mLevel != 2 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 5 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level3/version1/core"))
      {
        match = 1;
        if (mLevel != 3 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 1 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
      else if (!strcmp(ns->getURI(n).c_str(), 
                "http://www.sbml.org/sbml/level3/version2/core"))
      {
        match = 1;
        if (mLevel != 3 || !levelRead)
        {
          logError(MissingOrInconsistentLevel);
        }
        if (mVersion != 2 || !versionRead)
        {
          logError(MissingOrInconsistentVersion);
        }
        break;
      }
    }
    if (match == 0)
    {
      logError(InvalidNamespaceOnSBML);
    }
    else
    {
      mSBMLNamespaces->setLevel(mLevel);
      mSBMLNamespaces->setVersion(mVersion);
      setElementNamespace(mSBMLNamespaces->getURI());
    }

  }

  SBMLExtensionRegistry::getInstance().enableL2NamespaceForDocument(this);
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
/*
 * Subclasses should override this method to write their XML attributes
 * to the XMLOutputStream.  Be sure to call your parent's implementation
 * of this method as well.
 */
void
SBMLDocument::writeAttributes (XMLOutputStream& stream) const
{
  SBase::writeAttributes(stream);

  //
  // level: positiveInteger  { use="required" fixed="1" }  (L1v1)
  // level: positiveInteger  { use="required" fixed="2" }  (L2v1)
  //

  /* when a non xml model is read in level and version are set to 0
   * is we were for some obscure reason writing out the SBMLDocument that 
   * was created - we dont want to use l0v0
   */
  if (mLevel > 0)
  {
    stream.writeAttribute("level", mLevel);
  }
  else
  {
    stream.writeAttribute("level", getDefaultLevel());
  }

  //
  // version: positiveInteger  { use="required" fixed="1" }  (L1v1, L2v1)
  // version: positiveInteger  { use="required" fixed="2" }  (L1v2, L2v2)
  // version: positiveInteger  { use="required" fixed="3" }  (L2v3)
  //

  /* when a non xml model is read in level and version are set to 0
  * is we were for some obscure reason writing out the SBMLDocument that
  * was created - we dont want to use l0v0
  */
  if (mVersion > 0)
  {
    stream.writeAttribute("version", mVersion);
  }
  else
  {
    stream.writeAttribute("version", getDefaultVersion());
  }



  //
  // (EXTENSION)
  //
  SBase::writeExtensionAttributes(stream);

  //
  // required attributes of unknown packages
  //
  for (int i=0; i < mRequiredAttrOfUnknownPkg.getLength(); i++)
  {
    std::string prefix = mRequiredAttrOfUnknownPkg.getPrefix(i);
    std::string value  = mRequiredAttrOfUnknownPkg.getValue(i);
    stream.writeAttribute("required", prefix, value);
  }
}


/*
 *
 * Subclasses should override this method to write their xmlns attriubutes
 * (if any) to the XMLOutputStream.  Be sure to call your parent's implementation
 * of this method as well.
 *
 */
void
SBMLDocument::writeXMLNS (XMLOutputStream& stream) const
{
  /* when a non xml model is read in level and version are set to 0
  * is we were for some obscure reason writing out the SBMLDocument that
  * was created - we dont want to use l0v0
  */
  unsigned int level = mLevel;
  unsigned int version = mVersion;
  if (mLevel == 0 && mVersion == 0)
  {
    level = getDefaultLevel();
    version = getDefaultVersion();
  }
  // need to check that we have indeed a namespace set!
  XMLNamespaces * thisNs = this->getNamespaces();

  // the sbml namespace is missing - add it
  if (thisNs == NULL)
  {
    XMLNamespaces xmlns;
    xmlns.add(SBMLNamespaces::getSBMLNamespaceURI(level, version));

    mSBMLNamespaces->setNamespaces(&xmlns);
    thisNs = getNamespaces();
  }
  else if (thisNs->getLength() == 0)
  {
     thisNs->add(SBMLNamespaces::getSBMLNamespaceURI(level, version));
  }
  else
  {
    // check that there is an sbml namespace
    std::string sbmlURI = SBMLNamespaces::getSBMLNamespaceURI(level, version);
    std::string sbmlPrefix = thisNs->getPrefix(sbmlURI);
    if (thisNs->hasNS(sbmlURI, sbmlPrefix) == false)
    {
      // the sbml ns is not present
      std::string other = thisNs->getURI(sbmlPrefix);
      if (other.empty() == false)
      {
        // there is another ns with the prefix that the sbml ns expects to have
        //remove the this ns, add the sbml ns and 
        //add the new ns with a new prefix
        thisNs->remove(sbmlPrefix);
        thisNs->add(sbmlURI, sbmlPrefix);
        thisNs->add(other, "addedPrefix");
      }
      else
      {
        thisNs->add(sbmlURI, sbmlPrefix);
      }
    }
  }

  // we do not want to write the l2 layout ns on the top level
  XMLNamespaces * xmlns = thisNs->clone();
  if (xmlns != NULL) 
  {
    SBMLExtensionRegistry::getInstance().removeL2Namespaces(xmlns);

    stream << *(xmlns);
    delete xmlns;
  }
}
/** @endcond */


/** @cond doxygenLibsbmlInternal */
/*
 * Subclasses should override this method to write out their contained
 * SBML objects as XML elements.  Be sure to call your parent's
 * implementation of this method as well.
 */
void
SBMLDocument::writeElements (XMLOutputStream& stream) const
{
  SBase::writeElements(stream);
  if (mModel != NULL) mModel->write(stream);

  //
  // (EXTENSION)
  //
  SBase::writeExtensionElements(stream);
}


/*
 * Enables/Disables the given package with this element and child
 * elements (if any).
 * (This is an internal implementation for enablePackage function)
 */
void 
SBMLDocument::enablePackageInternal(const std::string& pkgURI, const std::string& pkgPrefix, bool flag)
{
  SBase::enablePackageInternal(pkgURI,pkgPrefix,flag);

  if (!flag)
  {
    //
    // disable the given package
    //

    mPkgUseDefaultNSMap.erase(pkgURI);

    /* before we remove the unknown package keep a copy
     * in case we try to re-enable it later
     */
    for (int i = 0; i < mRequiredAttrOfUnknownPkg.getLength(); i++)
    {
      if (pkgURI == mRequiredAttrOfUnknownPkg.getURI(i)
        && pkgPrefix == mRequiredAttrOfUnknownPkg.getPrefix(i))
      {
        mRequiredAttrOfUnknownDisabledPkg.add(
          mRequiredAttrOfUnknownPkg.getName(i), 
          mRequiredAttrOfUnknownPkg.getValue(i), pkgURI, pkgPrefix);
        mRequiredAttrOfUnknownPkg.remove(i);
        break;
      }
    }
  }
  else
  {
    /* check whether we are trying to reenable an unknown package
     * that we previously disabled
     */
    for (int i = 0; i < mRequiredAttrOfUnknownDisabledPkg.getLength(); i++)
    {
      if (pkgURI == mRequiredAttrOfUnknownDisabledPkg.getURI(i)
        && pkgPrefix == mRequiredAttrOfUnknownDisabledPkg.getPrefix(i))
      {
        mRequiredAttrOfUnknownPkg.add(
          mRequiredAttrOfUnknownDisabledPkg.getName(i), 
          mRequiredAttrOfUnknownDisabledPkg.getValue(i), pkgURI, pkgPrefix);
        mRequiredAttrOfUnknownDisabledPkg.remove(i);
        break;
      }
    }
  }

  if (mModel)
    mModel->enablePackageInternal(pkgURI,pkgPrefix,flag);
}
/** @endcond */



#endif /* __cplusplus */
/** @cond doxygenIgnored */
LIBSBML_EXTERN
SBMLDocument_t *
SBMLDocument_create ()
{
  try
  {
    SBMLDocument* obj = new SBMLDocument();
    return obj;
  }
  catch (SBMLConstructorException &)
  {
    return NULL;
  }
}


LIBSBML_EXTERN
SBMLDocument_t *
SBMLDocument_createWithLevelAndVersion (unsigned int level, unsigned int version)
{
  try
  {
    SBMLDocument* obj = new SBMLDocument(level, version);
    return obj;
  }
  catch (SBMLConstructorException &)
  {
    return NULL;
  }
}


LIBSBML_EXTERN
SBMLDocument_t *
SBMLDocument_createWithSBMLNamespaces (SBMLNamespaces_t *sbmlns)
{
  try
  {
    SBMLDocument* obj = new SBMLDocument(sbmlns);
    return obj;
  }
  catch (SBMLConstructorException &)
  {
    return NULL;
  }
}


LIBSBML_EXTERN
void
SBMLDocument_free (SBMLDocument_t *d)
{
  if (d != NULL)
    delete d;
  d = NULL;
}


LIBSBML_EXTERN
SBMLDocument_t *
SBMLDocument_clone (const SBMLDocument_t *d)
{
  return (d != NULL) ? static_cast<SBMLDocument_t*>( d->clone() ) : NULL;
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_getLevel (const SBMLDocument_t *d)
{
  return (d != NULL) ? d->getLevel() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_getVersion (const SBMLDocument_t *d)
{
  return (d != NULL) ? d->getVersion() : SBML_INT_MAX;
}


LIBSBML_EXTERN
int
SBMLDocument_isSetModel(const SBMLDocument_t *d)
{
  return (d != NULL) ? (int)d->isSetModel() : 0;
}


LIBSBML_EXTERN
Model_t *
SBMLDocument_getModel (SBMLDocument_t *d)
{
  return (d != NULL) ? d->getModel() : NULL;
}


LIBSBML_EXTERN
int
SBMLDocument_expandFunctionDefintions (SBMLDocument_t *d)
{
  return (d != NULL) ? 
    static_cast <int> (d->expandFunctionDefinitions()) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_expandInitialAssignments (SBMLDocument_t *d)
{
  return (d != NULL) ? 
    static_cast <int> (d->expandInitialAssignments()) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_setLevelAndVersion (  SBMLDocument_t *d
                                 , unsigned int    level
                                 , unsigned int    version )
{
  return (d != NULL) ? 
    static_cast <int> (d->setLevelAndVersion(level, version, true)) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_setLevelAndVersionStrict (  SBMLDocument_t *d
                                       , unsigned int    level
                                       , unsigned int    version )
{
  return (d != NULL) ? 
    static_cast <int> (d->setLevelAndVersion(level, version, true)) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_setLevelAndVersionNonStrict (  SBMLDocument_t *d
                                 , unsigned int    level
                                 , unsigned int    version )
{
  return (d != NULL) ? 
    static_cast <int> (d->setLevelAndVersion(level, version, false)) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_setModel (SBMLDocument_t *d, const Model_t *m)
{
  return (d != NULL) ? d->setModel(m) : LIBSBML_INVALID_OBJECT;
}


LIBSBML_EXTERN
Model_t *
SBMLDocument_createModel (SBMLDocument_t *d)
{
  return (d != NULL) ? d->createModel() : NULL;
}

LIBSBML_EXTERN
void 
SBMLDocument_setLocationURI (SBMLDocument_t *d, const char* location)
{
  if (d != NULL && location != NULL) d->setLocationURI(location);
}

LIBSBML_EXTERN
char*
SBMLDocument_getLocationURI(SBMLDocument_t *d)
{
  return (d != NULL) ? safe_strdup( d->getLocationURI().c_str() ) : NULL;
}


LIBSBML_EXTERN
void
SBMLDocument_setConsistencyChecks(SBMLDocument_t * d, 
                                  SBMLErrorCategory_t category,
                                  int apply)
{
  if (d != NULL)
    d->setConsistencyChecks(SBMLErrorCategory_t(category), apply);
}


LIBSBML_EXTERN
void
SBMLDocument_setConsistencyChecksForConversion(SBMLDocument_t * d, 
                                  SBMLErrorCategory_t category,
                                  int apply)
{
  if (d != NULL)
    d->setConsistencyChecksForConversion(SBMLErrorCategory_t(category), apply);
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_checkConsistency (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkConsistency() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_checkInternalConsistency (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkInternalConsistency() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL1Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL1Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL2v1Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL2v1Compatibility() : SBML_INT_MAX;
}



LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL2v2Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL2v2Compatibility() : SBML_INT_MAX;
}



LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL2v3Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL2v3Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL2v4Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL2v4Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL2v5Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL2v5Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int 
SBMLDocument_checkL3v1Compatibility (SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL3v1Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_checkL3v2Compatibility(SBMLDocument_t *d)
{
  return (d != NULL) ? d->checkL3v2Compatibility() : SBML_INT_MAX;
}


LIBSBML_EXTERN
const SBMLError_t *
SBMLDocument_getError (SBMLDocument_t *d, unsigned int n)
{
  return (d != NULL) ? d->getError(n) : NULL;
}

LIBSBML_EXTERN
const SBMLError_t *
SBMLDocument_getErrorWithSeverity(SBMLDocument_t *d, unsigned int n, unsigned int severity)
{
  return (d != NULL) ? d->getErrorWithSeverity(n, severity) : NULL;
}


LIBSBML_EXTERN
unsigned int
SBMLDocument_getNumErrors (const SBMLDocument_t *d)
{
  return (d != NULL) ? d->getNumErrors() : SBML_INT_MAX;
}

LIBSBML_EXTERN
unsigned int
SBMLDocument_getNumErrorsWithSeverity (const SBMLDocument_t *d, unsigned int severity)
{
   return (d != NULL) ? d->getNumErrors(severity) : SBML_INT_MAX;
}


LIBSBML_EXTERN
void
SBMLDocument_printErrors (SBMLDocument_t *d, FILE *stream)
{
  if (d == NULL) return;
  unsigned int numErrors = d->getNumErrors();

  if (numErrors > 0)
  {
    for (unsigned int n = 0; n < numErrors; n++)
    {
      XMLError_print(d->getError(n), stream);
    }
  }
}


unsigned int
SBMLDocument_getDefaultLevel ()
{
  return SBMLDocument::getDefaultLevel();
}


unsigned int
SBMLDocument_getDefaultVersion ()
{
  return SBMLDocument::getDefaultVersion();
}

LIBSBML_EXTERN
const XMLNamespaces_t *
SBMLDocument_getNamespaces(SBMLDocument_t *d)
{
  return (d != NULL) ? d->getNamespaces() : NULL;
}

LIBSBML_EXTERN
int
SBMLDocument_setSBMLNamespaces (SBMLDocument_t *d, SBMLNamespaces_t * sbmlns)
{
  return (d != NULL) ? static_cast<SBase*>(d)->setSBMLNamespaces(sbmlns) 
    : LIBSBML_INVALID_OBJECT;
}


LIBSBML_EXTERN
int
SBMLDocument_getPkgRequired (SBMLDocument_t *d, const char * package)
{
  return (d != NULL) ? static_cast<int>(d->getPkgRequired(package)) : 0;
}

LIBSBML_EXTERN
int
SBMLDocument_getPackageRequired (SBMLDocument_t *d, const char * package)
{
  return (d != NULL) ? static_cast<int>(d->getPackageRequired(package)) : 0;
}


LIBSBML_EXTERN
int
SBMLDocument_setPkgRequired (SBMLDocument_t *d, const char * package, int flag)
{
  return (d != NULL) ? d->setPkgRequired(package, flag) : LIBSBML_INVALID_OBJECT;
}

LIBSBML_EXTERN
int
SBMLDocument_setPackageRequired (SBMLDocument_t *d, const char * package, int flag)
{
  return (d != NULL) ? d->setPackageRequired(package, flag) : LIBSBML_INVALID_OBJECT;
}

LIBSBML_EXTERN
int
SBMLDocument_isSetPkgRequired (SBMLDocument_t *d, const char * package)
{
  return (d != NULL) ? static_cast<int>(d->isSetPkgRequired(package)) : 0;
}

LIBSBML_EXTERN
int
SBMLDocument_isSetPackageRequired (SBMLDocument_t *d, const char * package)
{
  return (d != NULL) ? static_cast<int>(d->isSetPackageRequired(package)) : 0;
}

LIBSBML_EXTERN
int
SBMLDocument_convert(SBMLDocument_t *d, const ConversionProperties_t* props)
{
  if (d == NULL || props == NULL) return LIBSBML_INVALID_OBJECT;
  return d->convert(*props);
}
/** @endcond */
LIBSBML_CPP_NAMESPACE_END
