#
# @file    TestSpeciesType.py
# @brief   SpeciesType unit tests
#
# @author  Akiya Jouraku (Python conversion)
# @author  Sarah Keating 
# 
# ====== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING ======
#
# DO NOT EDIT THIS FILE.
#
# This file was generated automatically by converting the file located at
# src/sbml/test/TestSpeciesType.c
# using the conversion program dev/utilities/translateTests/translateTests.pl.
# Any changes made here will be lost the next time the file is regenerated.
#
# -----------------------------------------------------------------------------
# This file is part of libSBML.  Please visit http://sbml.org for more
# information about SBML, and the latest version of libSBML.
#
# Copyright 2005-2010 California Institute of Technology.
# Copyright 2002-2005 California Institute of Technology and
#                     Japan Science and Technology Corporation.
# 
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation.  A copy of the license agreement is provided
# in the file named "LICENSE.txt" included with this software distribution
# and also available online as http://sbml.org/software/libsbml/license.html
# -----------------------------------------------------------------------------

import sys
import unittest
import libsbml


class TestSpeciesType(unittest.TestCase):

  global CT
  CT = None

  def setUp(self):
    self.CT = libsbml.SpeciesType(2,4)
    if (self.CT == None):
      pass    
    pass  

  def tearDown(self):
    _dummyList = [ self.CT ]; _dummyList[:] = []; del _dummyList
    pass  

  def test_SpeciesType_create(self):
    self.assertTrue( self.CT.getTypeCode() == libsbml.SBML_SPECIES_TYPE )
    self.assertTrue( self.CT.getMetaId() == "" )
    self.assertTrue( self.CT.getNotes() == None )
    self.assertTrue( self.CT.getAnnotation() == None )
    self.assertTrue( self.CT.getId() == "" )
    self.assertTrue( self.CT.getName() == "" )
    self.assertEqual( False, self.CT.isSetId() )
    self.assertEqual( False, self.CT.isSetName() )
    pass  

  def test_SpeciesType_createWithNS(self):
    xmlns = libsbml.XMLNamespaces()
    xmlns.add( "http://www.sbml.org", "testsbml")
    sbmlns = libsbml.SBMLNamespaces(2,2)
    sbmlns.addNamespaces(xmlns)
    object = libsbml.SpeciesType(sbmlns)
    self.assertTrue( object.getTypeCode() == libsbml.SBML_SPECIES_TYPE )
    self.assertTrue( object.getMetaId() == "" )
    self.assertTrue( object.getNotes() == None )
    self.assertTrue( object.getAnnotation() == None )
    self.assertTrue( object.getLevel() == 2 )
    self.assertTrue( object.getVersion() == 2 )
    self.assertTrue( object.getNamespaces() != None )
    self.assertTrue( object.getNamespaces().getLength() == 2 )
    _dummyList = [ object ]; _dummyList[:] = []; del _dummyList
    pass  

  def test_SpeciesType_free_NULL(self):
    _dummyList = [ None ]; _dummyList[:] = []; del _dummyList
    pass  

  def test_SpeciesType_setId(self):
    id =  "mitochondria";
    self.CT.setId(id)
    self.assertTrue(( id == self.CT.getId() ))
    self.assertEqual( True, self.CT.isSetId() )
    if (self.CT.getId() == id):
      pass    
    self.CT.setId(self.CT.getId())
    self.assertTrue(( id == self.CT.getId() ))
    self.CT.setId("")
    self.assertEqual( False, self.CT.isSetId() )
    if (self.CT.getId() != None):
      pass    
    pass  

  def test_SpeciesType_setName(self):
    name =  "My_Favorite_Factory";
    self.CT.setName(name)
    self.assertTrue(( name == self.CT.getName() ))
    self.assertEqual( True, self.CT.isSetName() )
    if (self.CT.getName() == name):
      pass    
    self.CT.setName(self.CT.getName())
    self.assertTrue(( name == self.CT.getName() ))
    self.CT.setName("")
    self.assertEqual( False, self.CT.isSetName() )
    if (self.CT.getName() != None):
      pass    
    pass  

  def test_SpeciesType_unsetName(self):
    self.CT.setName( "name")
    self.assertTrue((  "name"      == self.CT.getName() ))
    self.assertEqual( True, self.CT.isSetName() )
    self.CT.unsetName()
    self.assertEqual( False, self.CT.isSetName() )
    pass  

def suite():
  suite = unittest.TestSuite()
  suite.addTest(unittest.makeSuite(TestSpeciesType))

  return suite

if __name__ == "__main__":
  if unittest.TextTestRunner(verbosity=1).run(suite()).wasSuccessful() :
    sys.exit(0)
  else:
    sys.exit(1)

