/**
 * Filename    : TestSBMLFormatter.cpp
 * Description : SBMLFormatter unit tests
 * Author(s)   : SBW Development Group <sysbio-team@caltech.edu>
 * Organization: Caltech ERATO Kitano Systems Biology Project
 * Created     : 2003-03-07
 * Revision    : $Id$
 * Source      : $Source$
 *
 * Copyright 2002 California Institute of Technology and
 * Japan Science and Technology Corporation.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  The software and
 * documentation provided hereunder is on an "as is" basis, and the
 * California Institute of Technology and Japan Science and Technology
 * Corporation have no obligations to provide maintenance, support,
 * updates, enhancements or modifications.  In no event shall the
 * California Institute of Technology or the Japan Science and Technology
 * Corporation be liable to any party for direct, indirect, special,
 * incidental or consequential damages, including lost profits, arising
 * out of the use of this software and its documentation, even if the
 * California Institute of Technology and/or Japan Science and Technology
 * Corporation have been advised of the possibility of such damage.  See
 * the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * The original code contained here was initially developed by:
 *
 *     Ben Bornstein
 *     The Systems Biology Workbench Development Group
 *     ERATO Kitano Systems Biology Project
 *     Control and Dynamical Systems, MC 107-81
 *     California Institute of Technology
 *     Pasadena, CA, 91125, USA
 *
 *     http://www.cds.caltech.edu/erato
 *     mailto:sysbio-team@caltech.edu
 *
 * Contributor(s):
 */


#include "sbml/common.h"
#include "sbml/SBMLFormatter.hpp"


BEGIN_C_DECLS


/**
 * Wraps the string s in the appropriate XML boilerplate.
 */
#define XML_HEADER   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define wrapXML(s)   XML_HEADER s


MemBufFormatTarget *target;
SBMLFormatter      *formatter;


void
TestSBMLFormatter_setup (void)
{
  target    = new MemBufFormatTarget();
  formatter = new SBMLFormatter("UTF-8", target);
}


void
TestSBMLFormatter_teardown (void)
{
  delete formatter;
}


START_TEST (test_SBMLFormatter_SBMLDocument)
{
  SBMLDocument_t *d = SBMLDocument_createWith(1, 2);

  const char *s = wrapXML
  (
    "<sbml xmlns=\"http://www.sbml.org/sbml/level1\" "
    "level=\"1\" version=\"2\"/>\n"
  );


  *formatter << d;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  SBMLDocument_free(d);
}
END_TEST


START_TEST (test_SBMLFormatter_Model)
{
  Model_t    *m = Model_createWith("Branch");
  const char *s = wrapXML("<model name=\"Branch\"/>\n");


  *formatter << m;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  Model_free(m);
}
END_TEST


START_TEST (test_SBMLFormatter_Unit)
{
  Unit_t *u = Unit_createWith(UNIT_KIND_KILOGRAM, 2, -3);

  const char *s = wrapXML
  (
    "<unit kind=\"kilogram\" exponent=\"2\" scale=\"-3\"/>\n"
  );


  *formatter << u;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  Unit_free(u);
}
END_TEST


START_TEST (test_SBMLFormatter_Unit_defaults)
{
  Unit_t     *u = Unit_createWith(UNIT_KIND_KILOGRAM, 1, 0);
  const char *s = wrapXML("<unit kind=\"kilogram\"/>\n");


  *formatter << u;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  Unit_free(u);
}
END_TEST


START_TEST (test_SBMLFormatter_UnitDefinition)
{
  UnitDefinition_t *ud = UnitDefinition_createWith("mmls");
  const char       *s  = wrapXML("<unitDefinition name=\"mmls\"/>\n");


  *formatter << ud;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  UnitDefinition_free(ud);
}
END_TEST


START_TEST (test_SBMLFormatter_UnitDefinition_full)
{
  UnitDefinition_t *ud = UnitDefinition_createWith("mmls");

  const char *s  = wrapXML
  (
    "<unitDefinition name=\"mmls\">\n"
    "  <listOfUnits>\n"
    "    <unit kind=\"mole\" scale=\"-3\"/>\n"
    "    <unit kind=\"liter\" exponent=\"-1\"/>\n"
    "    <unit kind=\"second\" exponent=\"-1\"/>\n"
    "  </listOfUnits>\n"
    "</unitDefinition>\n"
  );


  UnitDefinition_addUnit( ud, Unit_createWith(UNIT_KIND_MOLE  ,  1, -3) );
  UnitDefinition_addUnit( ud, Unit_createWith(UNIT_KIND_LITER , -1,  0) );
  UnitDefinition_addUnit( ud, Unit_createWith(UNIT_KIND_SECOND, -1,  0) );

  *formatter << ud;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  UnitDefinition_free(ud);
}
END_TEST


START_TEST (test_SBMLFormatter_Compartment)
{
  Compartment_t *c = Compartment_createWith("A", 2.1, NULL, "B");

  const char *s = wrapXML
  (
    "<compartment name=\"A\" volume=\"2.1\" outside=\"B\"/>\n"
  );


  *formatter << c;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  Compartment_free(c);
}
END_TEST


START_TEST (test_SBMLFormatter_Compartment_annotation)
{
  Compartment_t *c = Compartment_createWith("A", 2.1, NULL, "B");

  const char *a =
    "<annotation xmlns:mysim=\"http://www.mysim.org/ns\">\n"
    "  <mysim:nodecolors mysim:bgcolor=\"green\" mysim:fgcolor=\"white\"/>\n"
    "  <mysim:timestamp>2000-12-18 18:31 PST</mysim:timestamp>\n"
    "</annotation>";

  const char *s = wrapXML
  (
    "<compartment name=\"A\" volume=\"2.1\" outside=\"B\">\n"
    "  <annotation xmlns:mysim=\"http://www.mysim.org/ns\">\n"
    "  <mysim:nodecolors mysim:bgcolor=\"green\" mysim:fgcolor=\"white\"/>\n"
    "  <mysim:timestamp>2000-12-18 18:31 PST</mysim:timestamp>\n"
    "</annotation>\n"
    "</compartment>\n"
  );


  SBase_setAnnotation((SBase_t *) c, a);
  *formatter << c;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL);

  Compartment_free(c);
}
END_TEST


START_TEST (test_SBMLFormatter_Species)
{
  Species_t *sp = Species_createWith("Ca2", "cell", 0.7, "mole", 1, 2);

  const char *s = wrapXML
  (
    "<species name=\"Ca2\" compartment=\"cell\" initialAmount=\"0.7\""
    " units=\"mole\" boundaryCondition=\"true\" charge=\"2\"/>\n"
  );

  *formatter << sp;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Species_free(sp);
}
END_TEST


START_TEST (test_SBMLFormatter_Species_L1v1)
{
  Species_t *sp = Species_createWith("Ca2", "cell", 0.7, "mole", 1, 2);

  const char *s = wrapXML
  (
    "<specie name=\"Ca2\" compartment=\"cell\" initialAmount=\"0.7\""
    " units=\"mole\" boundaryCondition=\"true\" charge=\"2\"/>\n"
  );


  *formatter << SBMLFormatter::Level1 << SBMLFormatter::Version1;
  *formatter << sp;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Species_free(sp);
}
END_TEST


START_TEST (test_SBMLFormatter_Species_defaults)
{
  Species_t *sp = Species_createWith("Ca2", "cell", 0.7, "mole", 0, 2);

  const char *s = wrapXML
  (
    "<species name=\"Ca2\" compartment=\"cell\" initialAmount=\"0.7\""
    " units=\"mole\" charge=\"2\"/>\n"
  );

  *formatter << sp;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Species_free(sp);
}
END_TEST


START_TEST (test_SBMLFormatter_Species_optional)
{
  Species_t  *sp = Species_create();
  const char *s  = wrapXML("<species name=\"Ca2\" initialAmount=\"0.7\"/>\n");


  Species_setName(sp, "Ca2");
  Species_setInitialAmount(sp, 0.7);

  *formatter << sp;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Species_free(sp);
}
END_TEST


START_TEST (test_SBMLFormatter_Parameter)
{
  Parameter_t *p = Parameter_createWith("Km1", 2.3, "second");

  const char *s = wrapXML
  (
    "<parameter name=\"Km1\" value=\"2.3\" units=\"second\"/>\n"
  );


  *formatter << p;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Parameter_free(p);
}
END_TEST


START_TEST (test_SBMLFormatter_AlgebraicRule)
{
  AlgebraicRule_t *ar = AlgebraicRule_createWith("x + 1");

  const char *s = wrapXML
  (
    "<algebraicRule formula=\"x + 1\"/>\n"
  );


  *formatter << ar;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  AlgebraicRule_free(ar);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesConcentrationRule)
{
  SpeciesConcentrationRule_t *scr;

  const char *s = wrapXML
  (
    "<speciesConcentrationRule "
    "formula=\"t * s\" type=\"rate\" species=\"s\"/>\n"
  );


  scr = SpeciesConcentrationRule_createWith("t * s", RULE_TYPE_RATE, "s");
  *formatter << scr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesConcentrationRule_free(scr);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesConcentrationRule_defaults)
{
  SpeciesConcentrationRule_t *scr;

  const char *s = wrapXML
  (
    "<speciesConcentrationRule formula=\"t * s\" species=\"s\"/>\n"
    
  );


  scr = SpeciesConcentrationRule_createWith("t * s", RULE_TYPE_SCALAR, "s");
  *formatter << scr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesConcentrationRule_free(scr);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesConcentrationRule_L1v1)
{
  SpeciesConcentrationRule_t *scr;

  const char *s = wrapXML
  (
    "<specieConcentrationRule formula=\"t * s\" specie=\"s\"/>\n"
  );


  scr = SpeciesConcentrationRule_createWith("t * s", RULE_TYPE_SCALAR, "s");

  *formatter << SBMLFormatter::Level1 << SBMLFormatter::Version1;
  *formatter << scr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesConcentrationRule_free(scr);
}
END_TEST


START_TEST (test_SBMLFormatter_CompartmentVolumeRule)
{
  CompartmentVolumeRule_t *cvr;

  const char *s = wrapXML
  (
    "<compartmentVolumeRule "
    "formula=\"v + s\" type=\"rate\" compartment=\"c\"/>\n"
  );


  cvr = CompartmentVolumeRule_createWith("v + s", RULE_TYPE_RATE, "c");
  *formatter << cvr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  CompartmentVolumeRule_free(cvr);
}
END_TEST


START_TEST (test_SBMLFormatter_CompartmentVolumeRule_defaults)
{
  CompartmentVolumeRule_t *cvr;

  const char *s = wrapXML
  (
    "<compartmentVolumeRule formula=\"v + s\" compartment=\"c\"/>\n"
  );


  cvr = CompartmentVolumeRule_createWith("v + s", RULE_TYPE_SCALAR, "c");
  *formatter << cvr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  CompartmentVolumeRule_free(cvr);
}
END_TEST


START_TEST (test_SBMLFormatter_ParameterRule)
{
  ParameterRule_t *pr;

  const char *s = wrapXML
  (
    "<parameterRule "
    "formula=\"p * t\" type=\"rate\" name=\"p\"/>\n"
  );


  pr = ParameterRule_createWith("p * t", RULE_TYPE_RATE, "p");
  *formatter << pr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  ParameterRule_free(pr);
}
END_TEST


START_TEST (test_SBMLFormatter_ParameterRule_defaults)
{
  ParameterRule_t *pr;

  const char *s = wrapXML
  (
    "<parameterRule formula=\"p * t\" name=\"p\"/>\n"
  );


  pr = ParameterRule_createWith("p * t", RULE_TYPE_SCALAR, "p");
  *formatter << pr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  ParameterRule_free(pr);
}
END_TEST


START_TEST (test_SBMLFormatter_Reaction)
{
  Reaction_t *r = Reaction_createWith("r", NULL, 0, 1);

  const char *s = wrapXML
  (
    "<reaction name=\"r\" reversible=\"false\" fast=\"true\"/>\n"
  );


  *formatter << r;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Reaction_free(r);
}
END_TEST


START_TEST (test_SBMLFormatter_Reaction_defaults)
{
  Reaction_t *r = Reaction_create();
  const char *s = wrapXML("<reaction name=\"r\"/>\n");


  Reaction_setName(r, "r");
  *formatter << r;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Reaction_free(r);
}
END_TEST


START_TEST (test_SBMLFormatter_Reaction_full)
{
  KineticLaw_t       *kl  = KineticLaw_create();
  Reaction_t         *r   = Reaction_createWith("v1", kl, 1, 0);
  SpeciesReference_t *srr = SpeciesReference_createWith("x0", 1, 1);
  SpeciesReference_t *srp = SpeciesReference_createWith("s1", 1, 1);

  const char *s = wrapXML
  (
    "<reaction name=\"v1\">\n"
    "  <listOfReactants>\n"
    "    <speciesReference species=\"x0\"/>\n"
    "  </listOfReactants>\n"
    "  <listOfProducts>\n"
    "    <speciesReference species=\"s1\"/>\n"
    "  </listOfProducts>\n"
    "  <kineticLaw formula=\"(vm * s1)/(km + s1)\"/>\n"
    "</reaction>\n"
  );


  KineticLaw_setFormula(kl, "(vm * s1)/(km + s1)");
  Reaction_addReactant(r, srr);
  Reaction_addProduct (r, srp);

  *formatter << r;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  Reaction_free(r);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesReference)
{
  SpeciesReference_t *sr = SpeciesReference_createWith("s", 3, 2);

  const char *s = wrapXML
  (
    "<speciesReference species=\"s\" stoichiometry=\"3\" denominator=\"2\"/>\n"
  );


  *formatter << sr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesReference_free(sr);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesReference_L1v1)
{
  SpeciesReference_t *sr = SpeciesReference_createWith("s", 3, 2);

  const char *s = wrapXML
  (
    "<specieReference specie=\"s\" stoichiometry=\"3\" denominator=\"2\"/>\n"
  );


  *formatter << SBMLFormatter::Level1 << SBMLFormatter::Version1;
  *formatter << sr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesReference_free(sr);
}
END_TEST


START_TEST (test_SBMLFormatter_SpeciesReference_defaults)
{
  SpeciesReference_t *sr = SpeciesReference_createWith("s", 1, 1);
  const char         *s  = wrapXML("<speciesReference species=\"s\"/>\n");


  *formatter << sr;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  SpeciesReference_free(sr);
}
END_TEST


START_TEST (test_SBMLFormatter_KineticLaw)
{
  KineticLaw_t *kl = KineticLaw_createWith("k * e", "seconds", "item");

  const char *s = wrapXML
  (
    "<kineticLaw formula=\"k * e\" timeUnits=\"seconds\" "
    "substanceUnits=\"item\"/>\n"
  );


  *formatter << kl;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  KineticLaw_free(kl);
}
END_TEST


START_TEST (test_SBMLFormatter_KineticLaw_ListOfParameters)
{
  KineticLaw_t *kl = KineticLaw_createWith("nk * e", "seconds", "item");

  const char *s = wrapXML
  (
    "<kineticLaw formula=\"nk * e\" timeUnits=\"seconds\" "
    "substanceUnits=\"item\">\n"
    "  <listOfParameters>\n"
    "    <parameter name=\"n\" value=\"1.2\"/>\n"
    "  </listOfParameters>\n"
    "</kineticLaw>\n"
  );


  KineticLaw_addParameter(kl, Parameter_createWith("n", 1.2, NULL));
  *formatter << kl;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  KineticLaw_free(kl);
}
END_TEST


START_TEST (test_SBMLFormatter_KineticLaw_ListOfParameters_notes)
{
  KineticLaw_t *kl = KineticLaw_createWith("nk * e", "seconds", "item");

  const char *s = wrapXML
  (
    "<kineticLaw formula=\"nk * e\" timeUnits=\"seconds\" "
    "substanceUnits=\"item\">\n"
    "  <notes>\n"
    "    This is a note.\n"
    "  </notes>\n"
    "  <listOfParameters>\n"
    "    <parameter name=\"n\" value=\"1.2\"/>\n"
    "  </listOfParameters>\n"
    "</kineticLaw>\n"
  );


  KineticLaw_addParameter(kl, Parameter_createWith("n", 1.2, NULL));
  SBase_setNotes((SBase_t*) kl, "This is a note.");

  *formatter << kl;

  fail_unless( !strcmp((char *) target->getRawBuffer(), s), NULL );

  KineticLaw_free(kl);
}
END_TEST


Suite *
create_suite_SBMLFormatter (void)
{
  Suite *suite = suite_create("SBMLFormatter");
  TCase *tcase = tcase_create("SBMLFormatter");


  tcase_add_checked_fixture(tcase,
                            TestSBMLFormatter_setup,
                            TestSBMLFormatter_teardown);
 
  tcase_add_test( tcase, test_SBMLFormatter_SBMLDocument                      );
  tcase_add_test( tcase, test_SBMLFormatter_Model                             );
  tcase_add_test( tcase, test_SBMLFormatter_Unit                              );
  tcase_add_test( tcase, test_SBMLFormatter_Unit_defaults                     );
  tcase_add_test( tcase, test_SBMLFormatter_UnitDefinition                    );
  tcase_add_test( tcase, test_SBMLFormatter_UnitDefinition_full               );
  tcase_add_test( tcase, test_SBMLFormatter_Compartment                       );
  tcase_add_test( tcase, test_SBMLFormatter_Compartment_annotation            );
  tcase_add_test( tcase, test_SBMLFormatter_Species                           );
  tcase_add_test( tcase, test_SBMLFormatter_Species_L1v1                      );
  tcase_add_test( tcase, test_SBMLFormatter_Species_defaults                  );
  tcase_add_test( tcase, test_SBMLFormatter_Species_optional                  );
  tcase_add_test( tcase, test_SBMLFormatter_Parameter                         );
  tcase_add_test( tcase, test_SBMLFormatter_AlgebraicRule                     );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesConcentrationRule          );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesConcentrationRule_defaults );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesConcentrationRule_L1v1     );
  tcase_add_test( tcase, test_SBMLFormatter_CompartmentVolumeRule             );
  tcase_add_test( tcase, test_SBMLFormatter_CompartmentVolumeRule_defaults    );
  tcase_add_test( tcase, test_SBMLFormatter_ParameterRule                     );
  tcase_add_test( tcase, test_SBMLFormatter_ParameterRule_defaults            );
  tcase_add_test( tcase, test_SBMLFormatter_Reaction                          );
  tcase_add_test( tcase, test_SBMLFormatter_Reaction_defaults                 );
  tcase_add_test( tcase, test_SBMLFormatter_Reaction_full                     );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesReference                  );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesReference_L1v1             );
  tcase_add_test( tcase, test_SBMLFormatter_SpeciesReference_defaults         );
  tcase_add_test( tcase, test_SBMLFormatter_KineticLaw                        );
  tcase_add_test( tcase, test_SBMLFormatter_KineticLaw_ListOfParameters       );
  tcase_add_test( tcase, test_SBMLFormatter_KineticLaw_ListOfParameters_notes );

  suite_add_tcase(suite, tcase);

  return suite;
}


END_C_DECLS
