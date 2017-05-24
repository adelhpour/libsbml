/**
 * @file ArraysSBMLError.h
 * @brief Definition of the ArraysSBMLError class.
 * @author SBMLTeam
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBML. Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright (C) 2013-2017 jointly by the following organizations:
 * 1. California Institute of Technology, Pasadena, CA, USA
 * 2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 * 3. University of Heidelberg, Heidelberg, Germany
 *
 * Copyright (C) 2009-2013 jointly by the following organizations:
 * 1. California Institute of Technology, Pasadena, CA, USA
 * 2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 *
 * Copyright (C) 2006-2008 by the California Institute of Technology,
 * Pasadena, CA, USA
 *
 * Copyright (C) 2002-2005 jointly by the following organizations:
 * 1. California Institute of Technology, Pasadena, CA, USA
 * 2. Japan Science and Technology Agency, Japan
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation. A copy of the license agreement is provided in the
 * file named "LICENSE.txt" included with this software distribution and also
 * available online as http://sbml.org/software/libsbml/license.html
 * ------------------------------------------------------------------------ -->
 */


#ifndef ArraysSBMLError_H__
#define ArraysSBMLError_H__




LIBSBML_CPP_NAMESPACE_BEGIN




BEGIN_C_DECLS


typedef enum
{
  ArraysUnknown                                               = 8010100
, ArraysNSUndeclared                                          = 8010101
, ArraysElementNotInNs                                        = 8010102
, ArraysDuplicateComponentId                                  = 8010301
, ArraysIdSyntaxRule                                          = 8010302
, ArraysAttributeRequiredMissing                              = 8010201
, ArraysAttributeRequiredMustBeBoolean                        = 8010202
, ArraysAttributeRequiredMustHaveValue                        = 8010203
, ArraysSBaseAllowedElements                                  = 8020206
, ArraysSBaseLOIndicesAllowedCoreElements                     = 8020110
, ArraysSBaseLODimensionsAllowedCoreElements                  = 8020102
, ArraysSBaseLOIndicesAllowedCoreAttributes                   = 8020113
, ArraysSBaseLODimensionsAllowedCoreAttributes                = 8020105
, ArraysIndexAllowedCoreAttributes                            = 8020301
, ArraysIndexAllowedCoreElements                              = 8020309
, ArraysIndexAllowedAttributes                                = 8020302
, ArraysIndexAllowedElements                                  = 8020306
, ArraysIndexReferencedAttributeMustBeString                  = 8020303
, ArraysIndexArrayDimensionMustBeUnInteger                    = 8020304
, ArraysDimensionAllowedCoreAttributes                        = 8020201
, ArraysDimensionAllowedCoreElements                          = 8020402
, ArraysDimensionAllowedAttributes                            = 8020202
, ArraysDimensionSizeMustBeSBase                              = 8020204
, ArraysDimensionArrayDimensionMustBeUnInteger                = 8020203
, ArraysDimensionNameMustBeString                             = 8020406
} ArraysSBMLErrorCode_t;


END_C_DECLS




LIBSBML_CPP_NAMESPACE_END




#endif /* !ArraysSBMLError_H__ */


