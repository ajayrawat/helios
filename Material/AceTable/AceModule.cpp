/*
 Copyright (c) 2012, Esteban Pellegrino
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "AceModule.hpp"
#include "AceReader/ACEReader.hpp"
#include "AceReader/NeutronTable.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

AceIsotope::AceIsotope(const Ace::ReactionContainer& _reactions, const ChildGrid* _child_grid) :
	reactions(_reactions), aweight(_reactions.awr()), temperature(_reactions.temp()), child_grid(_child_grid) {

	/* Total microscopic cross section of this isotope */
	CrossSection sigma_total = reactions.get_xs(1);

	/* Check if the isotope contains a fission cross section */
	fissile = reactions.check_all("18");
	/* Check for ith-chance fission */
	if(!fissile) fissile = reactions.check_all("19-21,38");

	/* Set the fission probability (in case the information is available) */
	if(fissile) {
		/* Resize the probability array */
		fission_prob.resize(sigma_total.size());
		/* Get fission cross section */
		CrossSection fission_xs = reactions.get_xs(18);
		/* Check size */
		assert(fission_xs.size() == sigma_total.size());
		/* Calculate the fission probabilities for each energy */
		for(size_t i = 0 ; i < sigma_total.size() ; ++i)
			fission_prob[i] = fission_xs[i] / sigma_total[i];
	}

	/* Set the absorption probability */
	CrossSection absorption_xs = reactions.get_xs(27);
	/* Check size */
	if(absorption_xs.size() != sigma_total.size()) {
		cout << reactions.name() << " st = " << sigma_total.size() << " sa = " << absorption_xs.size() << endl;
		exit(1);
	}
	/* Resize the probability array */
	absorption_prob.resize(sigma_total.size());

	/* Calculate the absorption probabilities for each energy */
	for(size_t i = 0 ; i < sigma_total.size() ; ++i)
		absorption_prob[i] = absorption_xs[i] / sigma_total[i];

}

double AceIsotope::getAbsorptionProb(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	return factor * (absorption_prob[idx + 1] - absorption_prob[idx]) + absorption_prob[idx];
}

double AceIsotope::getFissionProb(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	return factor * (fission_prob[idx + 1] - fission_prob[idx]) + fission_prob[idx];
}

void AceIsotope::print(std::ostream& out) const {
	out << "child grid size = " << setw(6) << child_grid->size() << " ; isotope = " <<  setw(9) << reactions.name()
		<< " ; awr = " << setw(9) << aweight << " ; temperature = " << temperature / Constant::boltz << " K ";
}

AceModule::AceModule(const std::vector<McObject*>& aceObjects, const McEnvironment* environment) : McModule(name(),environment) {
	/* Create master grid */
	master_grid = new MasterGrid();
	/* Loop over the definitions to create isotopes */
	for(vector<McObject*>::const_iterator it = aceObjects.begin() ; it != aceObjects.end() ; ++it) {
		/* Cast to AceObject */
		AceObject* ace_material = dynamic_cast<AceObject*>(*it);
		string isotope = ace_material->table_name;
		if (isotope_map.find(isotope) == isotope_map.end()) {
			/* Get the neutron table using the AceReader */
			NeutronTable* table = dynamic_cast<NeutronTable*>(AceReader::getTable(isotope));
			/* Create child grid */
			const ChildGrid* child_grid = master_grid->pushGrid(table->getEnergyGrid().begin(), table->getEnergyGrid().end());
			/* Create isotope and update the map */
			isotope_map[isotope] = new AceIsotope(table->getReactions(), child_grid);
			/* Delete table, we don't need it anymore */
			delete table;
		}
	}
	/* Setup master grid */
	master_grid->setup();
}

template<>
std::vector<AceIsotope*> AceModule::getObject<AceIsotope>(const UserId& id) const {
	map<std::string,AceIsotope*>::const_iterator iso = isotope_map.find(id);
	if(iso == isotope_map.end()) {
		throw AceError(id,"Isotope does not exist inside the ace module");
	} else {
		vector<AceIsotope*> v;
		v.push_back((*iso).second);
		return v;
	}
}

void AceModule::printIsotopes(std::ostream& out) const {
	out << "Ace Module  " << endl;
	out << " - Master grid size :" << master_grid->size() << endl;
	for(map<string,AceIsotope*>::const_iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		out << " - " << *(*it).second << endl;
}

AceModule::~AceModule() {
	/* Delete isotopes */
	for(map<string,AceIsotope*>::iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		delete (*it).second;
	/* Delete master grid */
	delete master_grid;
}

} /* namespace Helios */
