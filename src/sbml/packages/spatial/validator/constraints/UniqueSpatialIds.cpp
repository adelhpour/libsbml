/**
 * @cond doxygenLibsbmlInternal
 *
 * @file    UniqueSpatialIds.cpp
 * @brief   Unique values for spatial id
 * @author  Sarah Keating
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 * 
 * Copyright (C) 2013-2017 jointly by the following organizations:
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 *     3. University of Heidelberg, Heidelberg, Germany
 * 
 * Copyright (C) 2009-2013 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EMBL-EBI), Hinxton, UK
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution
 * and also available online as http://sbml.org/software/libsbml/license.html
 * ---------------------------------------------------------------------- -->*/

#include "UniqueSpatialIds.h"
#include <sbml/packages/spatial/extension/SpatialModelPlugin.h>
#include <sbml/packages/spatial/extension/SpatialCompartmentPlugin.h>

/** @cond doxygenIgnored */
using namespace std;
/** @endcond */

LIBSBML_CPP_NAMESPACE_BEGIN
#ifdef __cplusplus


/*
 * Creates a new UniqueSpatialIds with the given constraint id.
 */
UniqueSpatialIds::UniqueSpatialIds (unsigned int id, Validator& v) : TConstraint<Model>(id, v)
{
}


/*
 * Destroys this Constraint.
 */
UniqueSpatialIds::~UniqueSpatialIds ()
{
}


/*
 * Checks that all ids for some given subset of the Model adhere to this
 * Constraint.  Override the doCheck() method to define your own subset.
 */
void
UniqueSpatialIds::check_ (const Model& m, const Model& )
{
  doCheck(m);
}


/*
 * @return the typename of the given SBase object.
 */
const char*
UniqueSpatialIds::getTypename (const SBase& object)
{
  return SBMLTypeCode_toString( object.getTypeCode(), object.getPackageName().c_str() );
}


/*
 * Logs a message that the given @p id (and its corresponding object) have
 * failed to satisfy this constraint.
 */
void
UniqueSpatialIds::logIdConflict (const std::string& id, const SBase& object)
{
  logFailure(object, getMessage(id, object));
}

/*
 * Resets the state of this GlobalConstraint by clearing its internal
 * list of error messages.
 */
void
UniqueSpatialIds::reset ()
{
  mIdMap.clear();
}


/*
 * Checks that the id associated with the given object is unique.  If it
 * is not, logIdConflict is called.
 */
void
UniqueSpatialIds::doCheckId (const SBase& object)
{ 
  if (object.isSetId())
  {
    const string& id = object.getId();

    if (mIdMap.insert( make_pair(id, &object) ).second == false)
    {
      logIdConflict(id, object);
    }
  }
}


void
UniqueSpatialIds::logId (const SBase& object)
{ 
  if (object.isSetId())
  {
    const string& id = object.getId();

    mIdMap.insert( make_pair(id, &object) );
  }
}


/*
 * @return the error message to use when logging constraint violations.
 * This method is called by logFailure.
 *
 * Returns a message that the given @p id and its corresponding object are
 * in  conflict with an object previously defined.
 */
const string
UniqueSpatialIds::getMessage (const string& id, const SBase& object)
{
  IdObjectMap::iterator iter = mIdMap.find(id);


  if (iter == mIdMap.end())
  {
    return
      "Internal (but non-fatal) Validator error in "
      "UniqueSpatialIds::getMessage().  The SBML object with duplicate id was "
      "not found when it came time to construct a descriptive error message.";
  }


  ostringstream msg;
  const SBase&  previous = *(iter->second);


  //msg << getPreamble();

  //
  // Example message: 
  //
  // The <compartment> id 'cell' conflicts with the previously defined
  // <parameter> id 'cell' at line 10.
  //

  msg << "  The <" << object.getElementName()
      << "> id '" << id << "' conflicts with the previously defined <"
      << previous.getElementName()
      << "> id '" << id << "'";

  if (previous.getLine() != 0)
  {
    msg << " at line " << previous.getLine();
  }

  msg << '.';

  return msg.str();
}
/*
 * Checks that all ids on the following Spatial objects are unique:
 * .
 */
void
UniqueSpatialIds::doCheck (const Model& m)
{
  //createExistingMap(m);

  unsigned int n, size, j, num;

  const SpatialModelPlugin * modelPlug = 
    static_cast<const SpatialModelPlugin*>(m.getPlugin("spatial"));

  if (modelPlug->isSetGeometry())
  {
    const Geometry *g = modelPlug->getGeometry();
    doCheckId(*g);

    size = g->getNumAdjacentDomains();
    for (n = 0; n < size; n++)
    {
      doCheckId(*g->getAdjacentDomains(n));
    }

    size = g->getNumCoordinateComponents();
    for (n = 0; n < size; n++)
    {
      const CoordinateComponent *cc = g->getCoordinateComponent(n);
      doCheckId(*cc);
      doCheckId(*cc->getBoundaryMax());
      doCheckId(*cc->getBoundaryMin());
    }

    size = g->getNumDomainTypes();
    for (n = 0; n < size; n++)
    {
      doCheckId(*g->getDomainType(n));
    }

    size = g->getNumDomains();
    for (n = 0; n < size; n++)
    {
      doCheckId(*g->getDomain(n));
    }

    size = g->getNumGeometryDefinitions();
    for (n = 0; n < size; n++)
    {
      doCheckId(*g->getGeometryDefinition(n));
    }

    size = g->getNumSampledFields();
    for (n = 0; n < size; n++)
    {
      doCheckId(*g->getSampledField(n));
    }
  }

  size = m.getNumCompartments();
  for (n = 0; n < size; n++)
  {
    SpatialCompartmentPlugin *compPlug = (SpatialCompartmentPlugin*)
                             (m.getCompartment(n)->getPlugin("spatial"));

    if (compPlug->isSetCompartmentMapping())
    {
      doCheckId(*compPlug->getCompartmentMapping());
    }
  }

  //size = modelPlug->getNumFluxBounds();
  //for (n = 0; n < size; ++n) 
  //{
  //  doCheckId( *modelPlug->getFluxBound(n) );
  //}

  //size = modelPlug->getNumObjectives();
  //for (n = 0; n < size; ++n) 
  //{
  //  const Objective *obj = modelPlug->getObjective(n);
  //  doCheckId( *obj );
  //  num = obj->getNumFluxObjectives();
  //  for (j = 0; j < num; j++)
  //  {
  //    doCheckId(*obj->getFluxObjective(j));
  //  }
  //}

  reset();
}

/*
 * Checks that all ids on the following Model objects are unique:
 * FunctionDefinitions, Species, Compartments, global Parameters,
 * Reactions, and Events.
 */
void
UniqueSpatialIds::createExistingMap (const Model& m)
{
  // spatial ids have the SpId ns and so do not overlap with ids from core

  //unsigned int n, size, sr, sr_size;

  //logId( m );

  //size = m.getNumFunctionDefinitions();
  //for (n = 0; n < size; ++n) logId( *m.getFunctionDefinition(n) );

  //size = m.getNumCompartments();
  //for (n = 0; n < size; ++n) logId( *m.getCompartment(n) );

  //size = m.getNumSpecies();
  //for (n = 0; n < size; ++n) logId( *m.getSpecies(n) );

  //size = m.getNumParameters();
  //for (n = 0; n < size; ++n) logId( *m.getParameter(n) );

  //size = m.getNumReactions();
  //for (n = 0; n < size; ++n) 
  //{
  //  logId( *m.getReaction(n) );

  //  sr_size = m.getReaction(n)->getNumReactants();
  //  for (sr = 0; sr < sr_size; sr++)
  //  {
  //    logId(*m.getReaction(n)->getReactant(sr));
  //  }

  //  sr_size = m.getReaction(n)->getNumProducts();
  //  for (sr = 0; sr < sr_size; sr++)
  //  {
  //    logId(*m.getReaction(n)->getProduct(sr));
  //  }

  //  sr_size = m.getReaction(n)->getNumModifiers();
  //  for (sr = 0; sr < sr_size; sr++)
  //  {
  //    logId((SBase&)(*m.getReaction(n)->getModifier(sr)));
  //  }

  //}

  //size = m.getNumEvents();
  //for (n = 0; n < size; ++n) logId( *m.getEvent(n) );

  //size = m.getNumCompartmentTypes();
  //for (n = 0; n < size; ++n) logId( *m.getCompartmentType(n) );

  //size = m.getNumSpeciesTypes();
  //for (n = 0; n < size; ++n) logId( *m.getSpeciesType(n) );
}

#endif /* __cplusplus */
LIBSBML_CPP_NAMESPACE_END
/** @endcond */
