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

Cell_Definition motile_cell_1; 
Cell_Definition passive_cell;
Cell_Definition motile_cell_2;

void create_cell_types( void )
{
	// use the same random seed so that future experiments have the 
	// same initial histogram of oncoprotein, even if threading means 
	// that future division and other events are still not identical 
	// for all runs 

	SeedRandom( parameters.ints("random_seed") ); // or specify a seed here 

	// housekeeping 

	initialize_default_cell_definition();
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment ); 

	// Name the default cell type 

	cell_defaults.type = 3; 
	cell_defaults.name = "tumor cell"; 

	// set default cell cycle model 

	cell_defaults.functions.cycle_model = flow_cytometry_separated_cycle_model; 

	// set default_cell_functions; 

	cell_defaults.functions.update_phenotype = update_cell_and_death_parameters_O2_based; 

	// needed for a 2-D simulation: 

	/* grab code from heterogeneity */ 

	cell_defaults.functions.set_orientation = up_orientation; 
	cell_defaults.phenotype.geometry.polarity = 1.0;
	cell_defaults.phenotype.motility.restrict_to_2D = true; 

	// make sure the defaults are self-consistent. 

	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment );
	cell_defaults.phenotype.molecular.sync_to_microenvironment( &microenvironment );	
	cell_defaults.phenotype.sync_to_functions( cell_defaults.functions ); 

	// set the rate terms in the default phenotype 

	// first find index for a few key variables. 
	int apoptosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Apoptosis" );
	int necrosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Necrosis" );
	int oxygen_substrate_index = microenvironment.find_density_index( "oxygen" ); 

	int G0G1_index = flow_cytometry_separated_cycle_model.find_phase_index( PhysiCell_constants::G0G1_phase );
	int S_index = flow_cytometry_separated_cycle_model.find_phase_index( PhysiCell_constants::S_phase );

	// initially no necrosis 
	cell_defaults.phenotype.death.rates[necrosis_model_index] = 0.0; 

	// set oxygen uptake / secretion parameters for the default cell type 
	cell_defaults.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 10; 
	cell_defaults.phenotype.secretion.secretion_rates[oxygen_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[oxygen_substrate_index] = 38; 

	// add custom data here, if any 

	// Defining passive cells
	passive_cell = cell_defaults;
	passive_cell.type = 0;
	passive_cell.name = "passive cell";


	// make sure the new cell type has its own reference phenotype

	passive_cell.parameters.pReference_live_phenotype = &( passive_cell.phenotype );

	// must be completely immobile
	passive_cell.phenotype.motility.is_motile = false;

	// Set cell-cell adhesion to 0% of other cells
	passive_cell.phenotype.mechanics.cell_cell_adhesion_strength *= 0.0;
	// Set strong resistance to deformation since these are used to enforce confinement //10
	passive_cell.phenotype.mechanics.cell_cell_repulsion_strength = 10.0;

	// set parameter cell_radius 
	passive_cell.phenotype.geometry.radius = 10;
	passive_cell.phenotype.death.rates[apoptosis_model_index] = 0.0;
	// set oxygen uptake / secretion parameters for the default cell type
	passive_cell.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 0;
	passive_cell.phenotype.secretion.secretion_rates[oxygen_substrate_index] = 0;

	// Set proliferation to 10% of other cells.
	// Alter the transition rate from G0G1 state to S state
	passive_cell.phenotype.cycle.data.transition_rate(G0G1_index,S_index) *= 0.0;


	// Now, let's define another cell type. 
	// It's best to just copy the default and modify it. 

	// make this cell type randomly motile, less adhesive, greater survival, 
	// and less proliferative 

	motile_cell_1 = cell_defaults; 
	motile_cell_1.type = 1;
	motile_cell_1.name = "motile cell 1"; 

	motile_cell_2 = cell_defaults;
	motile_cell_2.type = 2;
	motile_cell_2.name ="motile cell 2";
	// make sure the new cell type has its own reference phenotype

	motile_cell_1.parameters.pReference_live_phenotype = &( motile_cell_1.phenotype ); 
	motile_cell_2.parameters.pReference_live_phenotype = &( motile_cell_2.phenotype );
	
	//cell_radii parameters
	motile_cell_1.phenotype.geometry.radius = parameters.doubles ("motile_cell_1_radius") ;
	motile_cell_2.phenotype.geometry.radius = parameters.doubles ("motile_cell_2_radius");
	
	// enable random motility 
	motile_cell_1.phenotype.motility.is_motile = true; 
	motile_cell_1.phenotype.motility.persistence_time = parameters.doubles( "motile_cell_1_persistence_time" ); // 15.0; 
	motile_cell_1.phenotype.motility.migration_speed = parameters.doubles( "motile_cell_1_migration_speed" ); // 0.25 micron/minute 
	motile_cell_1.phenotype.motility.migration_bias = parameters.doubles (" motile_cell_1_migration_bias");// initially set to 0 for completely random
	
	motile_cell_2.phenotype.motility.is_motile = true; 
	motile_cell_2.phenotype.motility.persistence_time = parameters.doubles( "motile_cell_2_persistence_time" ); // 15.0; 
	motile_cell_2.phenotype.motility.migration_speed = parameters.doubles( "motile_cell_2_migration_speed" ); // 0.25 micron/minute 
	motile_cell_2.phenotype.motility.migration_bias = parameters.doubles (" motile_cell_2_migration_bias");// initially set to 0 for completely random

	// Set cell-cell adhesion to 5% of other cells 
	motile_cell_1.phenotype.mechanics.cell_cell_adhesion_strength *= parameters.doubles( "motile_cell_1_relative_adhesion" ); // 0.05; 
	motile_cell_2.phenotype.mechanics.cell_cell_adhesion_strength *= parameters.doubles( "motile_cell_2_relative_adhesion" );
	// Set apoptosis to zero 
	motile_cell_1.phenotype.death.rates[apoptosis_model_index] = parameters.doubles( "motile_cell_1_apoptosis_rate" ); // 0.0; 
	motile_cell_2.phenotype.death.rates[apoptosis_model_index] = parameters.doubles( "motile_cell_1_apoptosis_rate" ); // 0.0;
	// Set proliferation rate 
	
	// Alter the transition rate from G0G1 state to S state
	motile_cell_1.phenotype.cycle.data.transition_rate(G0G1_index,S_index) *= parameters.doubles( "motile_cell_1_relative_cycle_entry_rate" );  //set to 0 for no division

	motile_cell_2.phenotype.cycle.data.transition_rate(G0G1_index,S_index) *= parameters.doubles( "motile_cell_2_relative_cycle_entry_rate" );  //set to 0 for no division


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
	// create cells evenly distributed within an ellipse


	const double xRadius = parameters.doubles("x_rad");
	const double yRadius = parameters.doubles("y_rad");
	const double motile_cell_1_density = parameters.doubles ("motile_cell_1_density"); //cell_1 density
	const double motile_cell_2_density = parameters.doubles ("motile_cell_2_density"); //cell_1 density
	
	const double total_density = motile_cell_1_density + motile_cell_2_density;
	if (total_density>= 1)
	{
		std::cout << "Warning: total cell density must not surpass 1!" << std::endl;
	}
/*	const double cell_ratio = parameters.doubles ("cell_ratio"); // motile_cell_1/motile_cell_2 ratio */
	const double ellipse_area = xRadius * yRadius * M_PI ;
	
	const double N1 = (double) motile_cell_1_density* ellipse_area/(M_PI*pow(motile_cell_1.phenotype.geometry.radius,2)); //total number of motile_1 cells
	const double N2 = (double) motile_cell_2_density*ellipse_area/(M_PI*pow(motile_cell_2.phenotype.geometry.radius,2)); //total number of motile_2 cells
	
	
/*	const int N = 100;  */
	const double passiveRad = passive_cell.phenotype.geometry.radius; // radius of passive cell
	const double passiveD = 2*passiveRad; // diameter of passive cell

	// domain for passive cells
	const int pWidth = xRadius + 6 * passiveRad;
	const int pHeight = yRadius + 6 * passiveRad;

	const double xR_squared = pow(xRadius,2);
	const double yR_squared = pow(yRadius,2);


	Cell* pC;

	// Generate passive cells confinement (at the contour of the ellipse)
	for (int w= -pWidth; w<pWidth; w+= passiveD){
		for (int h= -pHeight; h<pHeight; h+=passiveD){

			// Check if point is within ellipse

			double x_term = pow(w,2) / xR_squared;
			double y_term = pow(h,2) / yR_squared;

			// create only if outside
			if( x_term + y_term > 1){
				pC = create_cell(passive_cell);
				pC->assign_position( w, h, 0.0 );
				pC->is_movable=false;
			}


		}
	}


	// Now generate cells within the ellipse
	for (int i=0; i<N1; i++){
		double t = 2*M_PI * ((double) rand() / RAND_MAX) ;
		double d = sqrt(((double) rand() / RAND_MAX));
		double x = xRadius * d * cos(t);
		double y = yRadius * d * sin(t);

		pC = create_cell( motile_cell_1 );
		pC->assign_position( x, y, 0.0 );
	}

		// Now generate cells within the ellipse
	for (int j=0; j<N2; j++){
		double t = 2*M_PI * ((double) rand() / RAND_MAX) ;
		double d = sqrt(((double) rand() / RAND_MAX));
		double x = xRadius * d * cos(t);
		double y = yRadius * d * sin(t);

		pC = create_cell( motile_cell_2);
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
	if( (pCell->type == 0 ) )
	{
		output[0] = "grey";
		output[2] = "grey";
	}

	if ( (pCell -> type == 1))
	{
		output[0] = "blue";
		output[2] = "blue";
	}

	if ( (pCell -> type == 2))
	{
		output[0] = "red";
		output[2] = "red";
	}
	return output; 
}


//to test further : cell polarity
