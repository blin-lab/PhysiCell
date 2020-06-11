/*
###############################################################################
# If you use PhysiCell in your project, please cite PhysiCell and the version #
# number, such as below:                                                      #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1].    #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# See VERSION.txt or call get_PhysiCell_version() to get the current version  #
#     x.y.z. Call display_citations() to get detailed information on all cite-#
#     able software used in your PhysiCell application.                       #
#                                                                             #
# Because PhysiCell extensively uses BioFVM, we suggest you also cite BioFVM  #
#     as below:                                                               #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1],    #
# with BioFVM [2] to solve the transport equations.                           #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# [2] A Ghaffarizadeh, SH Friedman, and P Macklin, BioFVM: an efficient para- #
#     llelized diffusive transport solver for 3-D biological simulations,     #
#     Bioinformatics 32(8): 1256-8, 2016. DOI: 10.1093/bioinformatics/btv730  #
#                                                                             #
###############################################################################
#                                                                             #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)     #
#                                                                             #
# Copyright (c) 2015-2018, Paul Macklin and the PhysiCell Project             #
# All rights reserved.                                                        #
#                                                                             #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions are met: #
#                                                                             #
# 1. Redistributions of source code must retain the above copyright notice,   #
# this list of conditions and the following disclaimer.                       #
#                                                                             #
# 2. Redistributions in binary form must reproduce the above copyright        #
# notice, this list of conditions and the following disclaimer in the         #
# documentation and/or other materials provided with the distribution.        #
#                                                                             #
# 3. Neither the name of the copyright holder nor the names of its            #
# contributors may be used to endorse or promote products derived from this   #
# software without specific prior written permission.                         #
#                                                                             #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   #
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
# POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                             #
###############################################################################
 */

#include "./custom.h"

// declare cell definitions here 

Cell_Definition motile_cell; 
Cell_Definition passive_cell;


void create_cell_types( void )
{
	// use the same random seed so that future experiments have the 
	// same initial histogram of oncoprotein, even if threading means 
	// that future division and other events are still not identical 
	// for all runs 

	SeedRandom( parameters.ints("random_seed") ); // or specify a seed here 

	// housekeeping 
	initialize_default_cell_definition();


	// Name the default cell type 
	cell_defaults.type = 0; 
	cell_defaults.name = "secrete and sense cell";


	// We will use the simplest cell cycle model here
	cell_defaults.functions.cycle_model = live;


	// needed for a 2-D simulation: 
	/* grab code from heterogeneity */
	cell_defaults.functions.set_orientation = up_orientation; 
	cell_defaults.phenotype.geometry.polarity = 1.0;
	cell_defaults.phenotype.motility.restrict_to_2D = true; 

	//added this to see if enables chemotaxis
	cell_defaults.phenotype.motility.is_motile = true;

	// make sure the defaults are self-consistent. 
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment );
	cell_defaults.phenotype.molecular.sync_to_microenvironment( &microenvironment );	
	cell_defaults.phenotype.sync_to_functions( cell_defaults.functions ); 


	// No apoptosis
	int apoptosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Apoptosis" );
	cell_defaults.phenotype.death.rates[apoptosis_model_index] = 0.0;


	// set secretion rates
	int chemokine_substrate_index = microenvironment.find_density_index( "chemokine" );
	// set chemokine uptake / secretion parameters for the default cell type
	cell_defaults.phenotype.secretion.uptake_rates[chemokine_substrate_index] = parameters.doubles("chemokine_uptake_rate");
	cell_defaults.phenotype.secretion.secretion_rates[chemokine_substrate_index] = parameters.doubles("chemokine_secretion_rate");
	cell_defaults.phenotype.secretion.saturation_densities[chemokine_substrate_index] = 38;

	// Adds a membrane that prevents cells from exiting the space
	//cell_defaults.functions.calculate_distance_to_membrane = distance_to_membrane_ellipse;
	//cell_defaults.functions.add_cell_basement_membrane_interactions = standard_add_basement_membrane_interactions;
	//cell_defaults.phenotype.mechanics.cell_BM_repulsion_strength = 50.0;

	//This needs to be equal to chemotaxis function?
	//cell_defaults.functions.update_migration_bias;

	// Need to enable chemotaxis?
	cell_defaults.phenotype.motility.chemotaxis_index = chemokine_substrate_index;
	//(1 to go up gradient, -1 to go down gradient)
	cell_defaults.phenotype.motility.chemotaxis_direction = 1;

	//added in speed parameter
	cell_defaults.phenotype.motility.migration_speed = parameters.doubles("chemokine_cell_migration_speed");




	// Defining motile_cells

	motile_cell = cell_defaults;
	motile_cell.type = 1;
	motile_cell.name = "motile cell";

	// make sure the new cell type has its own reference phenotype
	motile_cell.parameters.pReference_live_phenotype = &( motile_cell.phenotype );

	// enable random motility
	motile_cell.phenotype.motility.is_motile = true;
	motile_cell.phenotype.motility.persistence_time = parameters.doubles( "motile_cell_persistence_time" ); // 15.0;
	motile_cell.phenotype.motility.migration_speed = parameters.doubles( "motile_cell_migration_speed" ); // 0.25 micron/minute
	motile_cell.phenotype.motility.migration_bias = 0.0;// completely random

	// Set cell-cell adhesion to 5% of other cells
	// motile_cell.phenotype.mechanics.cell_cell_adhesion_strength *= parameters.doubles( "motile_cell_relative_adhesion" ); // 0.05;
	// motile_cell.phenotype.mechanics.cell_cell_repulsion_strength *= 50;




	// Defining passive cells
	passive_cell = cell_defaults;
	passive_cell.type = 2;
	passive_cell.name = "passive cell";

	// make sure the new cell type has its own reference phenotype
	passive_cell.parameters.pReference_live_phenotype = &( passive_cell.phenotype );

	// must be completely immobile
	passive_cell.phenotype.motility.is_motile = false;

	// Set cell-cell adhesion to 0% of other cells
	passive_cell.phenotype.mechanics.cell_cell_adhesion_strength *= 0.0;

	// Set strong resistance to deformation since these are used to enforce confinement
	passive_cell.phenotype.mechanics.cell_cell_repulsion_strength = 50.0;

	// Adjust cycle
	passive_cell.phenotype.geometry.radius = parameters.doubles("passive_cell_radius");

	// set oxygen uptake / secretion parameters for the default cell type
	passive_cell.phenotype.secretion.uptake_rates[chemokine_substrate_index] = 0;
	passive_cell.phenotype.secretion.secretion_rates[chemokine_substrate_index] = 0;

	// Set proliferation to 0
	passive_cell.phenotype.cycle.data.transition_rate(0,0) = 0.0;






	build_cell_definitions_maps(); 
	display_cell_definitions( std::cout ); 

	return; 
}





void setup_microenvironment( void )
{


	// make sure to override and go back to 2D 
	if( default_microenvironment_options.simulate_2D == false )
	{
		std::cout << "Warning: overriding XML config option and setting to 2D!" << std::endl; 
		default_microenvironment_options.simulate_2D = true; 
	}


	// put any custom code to set non-homogeneous initial conditions or 
	// extra Dirichlet nodes here. 

	// initialize BioFVM 

	initialize_microenvironment(); 	

	return; 
}




void setup_tissue( void )
{

	// create cells randomly distributed within an ellipse

	const double xRadius = parameters.doubles("geom_x");
	const double yRadius = parameters.doubles("geom_y");
	const int N = 200;
	const double passiveRad = parameters.doubles("passive_cell_radius"); // radius of passive cell
	const double passiveD = passiveRad * 2; // diameter of passive cell

	// domain for passive cells
	const int pWidth = xRadius + 3 * passiveD;
	const int pHeight = yRadius + 3 * passiveD;

	const double xR_squared = xRadius * xRadius;
	const double yR_squared = yRadius * yRadius;


	Cell* pC;


	// Generate passive cells confinement (at the contour of the ellipse)
	for (int w= -pWidth; w<pWidth; w+= passiveD){
		for (int h= -pHeight; h<pHeight; h+=passiveD){


			// Check if point is within ellipse

			double x_term = w*w / xR_squared;
			double y_term = h*h / yR_squared;

			// create only if outside
			if( x_term + y_term > 1){
				pC = create_cell(passive_cell);
				pC->is_movable = false;
				pC->assign_position( w, h, 0.0 );
			}


		}
	}

//	const double M_PI = 3.14159265359;
#define M_PI 3.14159265358979323846264338327

	// generate secrete and sense cells within the ellipse
	for (int i=0; i<(N*0.05); i++){
		double t = 2*M_PI * ((double) rand() / RAND_MAX) ;
		double d = sqrt(((double) rand() / RAND_MAX));
		double x = xRadius * d * cos(t);
		double y = yRadius * d * sin(t);

		pC = create_cell( cell_defaults );
		pC->assign_position( x, y, 0.0 );
	}

	// generate motile cells within the ellipse
	for (int i=0; i<(N*0.95); i++){
			double t = 2*M_PI * ((double) rand() / RAND_MAX) ;
			double d = sqrt(((double) rand() / RAND_MAX));
			double x = xRadius * d * cos(t);
			double y = yRadius * d * sin(t);

			pC = create_cell( motile_cell );
			pC->assign_position( x, y, 0.0 );
		}



	return; 
}





std::vector<std::string> my_coloring_function( Cell* pCell )
{
	// start with flow cytometry coloring 

	std::vector<std::string> output = false_cell_coloring_cytometry(pCell); 

	//if( pCell->phenotype.death.dead == false && pCell->type == 2 )
	//{
	//	output[0] = "black";
	//	output[2] = "black";
	//}
	if( pCell->type == 2 )
	{
		output[0] = "grey";
		output[2] = "grey";
	}
	else if( pCell->type == 1 )
	{
		output[0] = "blue";
		output[2] = "blue";
	}
	else if( pCell->type == 0 )
	{
		output[0] = "red";
		output[2] = "red";
	}

	return output; 
}








double distance_to_membrane_ellipse(Cell* pCell, Phenotype& phenotype, double dummy)
{

	const double epsillon= 1e-7;
	const double xRadius = 400;
	const double yRadius = 100;

	//Note that this function assumes that ellipse center is located at <0, 0, 0>

	// find the angle that satisfies : origin to cell is perpendicular to the tangent of ellipse
	double t = atan2(xRadius * pCell->position[0], yRadius * pCell->position[1]);

	// get the coordinate at this angle
	double x = xRadius * cos(t);
	double y = yRadius * sin(t);

	// Now compute the distance from the membrane
	double d = dist(pCell->position, {x,y,0.0});

	// Adjust displacement to prevent cells from exiting the confined space
	double distance_to_origin= dist(pCell->position, {0.0,0.0,0.0});  // distance to the origin
	distance_to_origin = std::max(distance_to_origin, epsillon); // prevents div by zero

	pCell->displacement[0] = -pCell->position[0]/ distance_to_origin;
	pCell->displacement[1] = -pCell->position[1]/ distance_to_origin;


	return d;

}







