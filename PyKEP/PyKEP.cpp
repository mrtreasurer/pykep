/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/module.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/self.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/utility.hpp>

#include "../src/keplerian_toolbox.h"
#include "boost_python_container_conversions.h"
//#include "exceptions.h"
//#include "../utils.h"

using namespace boost::python;

static inline tuple planet_get_eph(const kep_toolbox::planet &p, const kep_toolbox::epoch &when)
{
	kep_toolbox::array3D r, v;
	p.get_eph(when,r,v);
	return boost::python::make_tuple(r,v);
}

#define get_constant(arg) \
static inline double get_##arg() \
{ \
	return ASTRO_##arg; \
}

get_constant(AU);
get_constant(MU_SUN);
get_constant(EARTH_VELOCITY);
get_constant(DEG2RAD);
get_constant(RAD2DEG);
get_constant(DAY2SEC);
get_constant(SEC2DAY);
get_constant(DAY2YEAR);
get_constant(G0);

BOOST_PYTHON_MODULE(_PyKEP) {
	//Disable docstring c++ signature to allow sphinx autodoc to work properly
	docstring_options doc_options;
	doc_options.disable_signatures();
  
	//Exposing the arrays and vectors into python tuples
	to_tuple_mapping<kep_toolbox::array6D>();
	from_python_sequence<kep_toolbox::array6D,fixed_size_policy>();
	to_tuple_mapping<kep_toolbox::array3D>();
	from_python_sequence<kep_toolbox::array3D,fixed_size_policy>();
	to_tuple_mapping<std::vector<kep_toolbox::array3D> >();
	from_python_sequence<std::vector<kep_toolbox::array3D>,variable_capacity_policy>();

	//Constants.
#define expose_constant(arg) \
def("_get_"#arg,&get_##arg);
	expose_constant(AU);
	expose_constant(MU_SUN);
	expose_constant(EARTH_VELOCITY);
	expose_constant(DEG2RAD);
	expose_constant(RAD2DEG);
	expose_constant(DAY2SEC);
	expose_constant(SEC2DAY);
	expose_constant(DAY2YEAR);
	expose_constant(G0);

	//Exposing enums
	enum_<kep_toolbox::epoch::type>("_epoch_type",
			"Defines a julian date type exposing the corresponding c++ enum\n\n"
			"One of\n\n"
			"* PyKEP.epoch.epoch_type.JD\n"
			"* PyKEP.epoch.epoch_type.MJD\n"
			"* PyKEP.epoch.epoch_type.MJD2000\n"
		)
		.value("MJD", kep_toolbox::epoch::MJD)
		.value("MJD2000", kep_toolbox::epoch::MJD2000)
		.value("JD", kep_toolbox::epoch::JD);
	
	// Epoch class
	class_<kep_toolbox::epoch>("epoch","Represents a precise point in time. Internal handling is made using boost::posix_time_ptime classes",
		init<const double &,kep_toolbox::epoch::type>(
			"epoch(jd, jd_type)\n\n"
			"- jd: a julian date\n"
			"- jd_type: julian date type\n\n"
			"Example::\n\n"
			"  e = epoch(0,PyKEP.epoch.epoch_type.MJD2000)\n"
		))
		.def("jd",&kep_toolbox::epoch::jd,
			"Returns the Julian Date\n\n"
			"Example::\n\n"
			"  jd = e.jd()"
		)
		.def("mjd",&kep_toolbox::epoch::mjd,
			"Returns the Modified Julian Date\n\n"
			"Example::\n\n"
			"  jd = e.mjd()"
		)
		.def("mjd2000",&kep_toolbox::epoch::mjd2000,
			"Returns the Modifeid Julian Date 2000\n\n"
			"Example::\n\n"
			"  jd = e.mjd2000()"
		)
		.def(repr(self));
		
	// Epoch constructors helpers
	def("epoch_from_string",&kep_toolbox::epoch_from_string,
		"Returns a :py:class:`PyKEP.epoch` object constructed from a from a delimited string containing a date."
		"Excess digits in fractional seconds will be dropped. Ex: '1:02:03.123456999' => '1:02:03.123456'."
		"This behavior depends on the precision defined in astro_constant.h used to compile.\n\n"
		"NOTE: The function is based on the corresponding `boost date_time library function <http://www.boost.org/doc/libs/1_44_0/doc/html/date_time/posix_time.html#ptime_from_string>`_\n\n"
		"Example::\n\n"
		"  e = PyKEP.epoch_from_string('2002-01-20 23:59:54.003')"
	);

	def("epoch_from_iso_string",&kep_toolbox::epoch_from_iso_string,
		"Returns a :py:class:`PyKEP.epoch` object constructed from a from a non delimited iso form string containing a date.\n\n"
		"NOTE: The function is based on the corresponding `boost date_time library function <http://www.boost.org/doc/libs/1_44_0/doc/html/date_time/posix_time.html#ptime_from_string>`_\n\n"
		"Example::\n\n"
		"  e = PyKEP.epoch_from_iso_string('20020120T235954')"
	);

	// Base planet class.
	class_<kep_toolbox::planet,boost::noncopyable>("_planet",no_init)
		.def("eph",&planet_get_eph)
		.def(repr(self))
		.def("elements",&kep_toolbox::planet::get_elements);
	
	// Solar system planet.
	class_<kep_toolbox::asteroid_gtoc5,bases<kep_toolbox::planet> >("asteroid_gtoc5",init<optional<const int &> >());
	
	register_ptr_to_python<kep_toolbox::planet_ptr>();

	// Lambert.
	class_<kep_toolbox::lambert_problem>("lambert_problem","Represents a multiple revolution Lambert's problem",
		init<const kep_toolbox::array3D &, const kep_toolbox::array3D &, const double &, optional<const double &, const int &> >(
			"lambert_problem(r1,r2,t, mu = 1, cw = False)\n\n"
			"- r1: starting position (x1,y1,z1)\n"
			"- r2: 3D final position (x2,y2,z2)\n"
			"- t: time of flight\n"
			"- mu: gravitational parameter\n"
			"- cw: True for clock-wise motion, False otherwise\n\n"
			"NOTE: Units need to be consistent.\n\n"
			"NOTE: The multirev Lambert's problem will be solved upon construction and its solution stored in data members.\n\n"
			"Example (non-dimensional units used)::\n\n"
			"  l = lambert_problem([1,0,0],[0,1,0],5 * pi / 2. )"
		))
		.def("get_v1",&kep_toolbox::lambert_problem::get_v1,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r1 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v1 for the 0 revs solution)::\n\n"
			"  v10 = l.get_v1()[0]"
		)
		.def("get_v2",&kep_toolbox::lambert_problem::get_v2,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r2 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v2 for the 0 revs solution)::\n\n"
			"  v20 = l.get_v2()[0]"			
		)
		.def("get_a",&kep_toolbox::lambert_problem::get_a,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the semi-major axes of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts a for the 0 revs solution)::\n\n"
			"  a0 = l.get_a()[0]"			
		)
		.def("get_p",&kep_toolbox::lambert_problem::get_p,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the semilatus rectum (parameter) of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts p for the 0 revs solution)::\n\n"
			"  p0 = l.get_p()[0]"
		)
		.def("get_iters",&kep_toolbox::lambert_problem::get_iters,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the number of iterations employed to compute each solution to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts the number of iterations employed for the 0 revs solution)::\n\n"
			"  p0 = l.get_iters()[0]"
		)
		.def("is_reliable",&kep_toolbox::lambert_problem::is_reliable,
			"Returns True if all solutions to the Lambert's problem have been solved using less iterations than the maximum number of iterations allowed. This is set at compile time in the c++ header file astro_constants.h\n\n" 
			"Example::\n\n"
			"  if l.is_reliable():\n"
			"    n_sol = len(l.get_a())"
		)
		.def("get_Nmax",&kep_toolbox::lambert_problem::get_Nmax,
			"Returns the maximum number of revolutions for which a solution exists. The total number of solution to the Lambert's problem will thus be n_sol = Nmax*2 + 1\n\n"
			"Example::\n\n"
			"  Nmax = l.get_Nmax()\n"
			"  n_sol = Nmax*2+1"
		)
		.def(repr(self));
}
