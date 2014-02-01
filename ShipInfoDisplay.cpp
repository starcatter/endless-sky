/* ShipInfoDisplay.cpp
Michael Zahniser, 24 Jan 2014

Function definitions for the ShipInfoDisplay class.
*/

#include "ShipInfoDisplay.h"

#include "Color.h"
#include "Font.h"
#include "FontSet.h"
#include "Outfit.h"
#include "Ship.h"

#include <cmath>
#include <map>
#include <sstream>

using namespace std;

namespace {
	static const int WIDTH = 250;
	
	// Round a value to three 
	string Round(double value)
	{
		ostringstream out;
		if(value >= 1000. || value <= -1000.)
			out << round(value);
		else
		{
			out.precision(3);
			out << value;
		}
		return out.str();
	}
	
	Point Draw(Point point, const vector<string> &labels, const vector<string> &values)
	{
		// Use additive color mixing.
		Color labelColor(.5, 0.);
		Color valueColor(.8, 0.);
		const Font &font = FontSet::Get(14);
		
		// Use 10-pixel margins on both sides.
		point.X() += 10.;
		for(unsigned i = 0; i < labels.size() && i < values.size(); ++i)
		{
			if(labels[i].empty())
			{
				point.Y() += 10.;
				continue;
			}
		
			font.Draw(labels[i], point,
				(values[i].empty() ? valueColor : labelColor).Get());
			Point align(WIDTH - 20 - font.Width(values[i]), 0.);
			font.Draw(values[i], point + align, valueColor.Get());
			point.Y() += 20.;
		}
		return point;
	}
}



ShipInfoDisplay::ShipInfoDisplay()
	: descriptionHeight(0), attributesHeight(0), outfitsHeight(0),
	saleHeight(0), maximumHeight(0)
{
}



ShipInfoDisplay::ShipInfoDisplay(const Ship &ship)
{
	Update(ship);
}



// Call this every time the ship changes.
void ShipInfoDisplay::Update(const Ship &ship)
{
	UpdateDescription(ship);
	UpdateAttributes(ship);
	UpdateOutfits(ship);
	
	maximumHeight = max(descriptionHeight, max(attributesHeight, outfitsHeight));
}



// Get the panel width.
int ShipInfoDisplay::PanelWidth() const
{
	return WIDTH;
}



// Get the height of each of the three panels.
int ShipInfoDisplay::MaximumHeight() const
{
	return maximumHeight;
}



int ShipInfoDisplay::DescriptionHeight() const
{
	return descriptionHeight;
}



int ShipInfoDisplay::AttributesHeight() const
{
	return attributesHeight;
}



int ShipInfoDisplay::OutfitsHeight() const
{
	return outfitsHeight;
}



int ShipInfoDisplay::SaleHeight() const
{
	return saleHeight;
}



// Draw each of the panels.
void ShipInfoDisplay::DrawDescription(const Point &topLeft) const
{
	description.Draw(topLeft + Point(10., 10.), Color(.5, 0.));
}



void ShipInfoDisplay::DrawAttributes(const Point &topLeft) const
{
	Point point = Draw(topLeft, attributeLabels, attributeValues);
	
	// Use additive color mixing.
	Color labelColor(.5, 0.);
	Color valueColor(.8, 0.);
	const Font &font = FontSet::Get(14);
		
	point.Y() += 10.;
	static const int ENERGY_COL = WIDTH - 100;
	static const int HEAT_COL = WIDTH - 20;
	font.Draw("energy", point + Point(ENERGY_COL - font.Width("energy"), 0.), labelColor.Get());
	font.Draw("heat", point + Point(HEAT_COL - font.Width("heat"), 0.), labelColor.Get());
	
	for(unsigned i = 0; i < tableLabels.size(); ++i)
	{
		point.Y() += 20.;
		font.Draw(tableLabels[i], point, labelColor.Get());
		Point energyAlign(ENERGY_COL - font.Width(energyTable[i]), 0.);
		font.Draw(energyTable[i], point + energyAlign, valueColor.Get());
		Point heatAlign(HEAT_COL - font.Width(heatTable[i]), 0.);
		font.Draw(heatTable[i], point + heatAlign, valueColor.Get());
	}
}



void ShipInfoDisplay::DrawOutfits(const Point &topLeft) const
{
	Draw(topLeft, outfitLabels, outfitValues);
}



void ShipInfoDisplay::DrawSale(const Point &topLeft) const
{
	Draw(topLeft, saleLabels, saleValues);
}



void ShipInfoDisplay::UpdateDescription(const Ship &ship)
{
	description.SetAlignment(WrappedText::JUSTIFIED);
	description.SetWrapWidth(WIDTH - 20);
	description.SetFont(FontSet::Get(14));
	
	description.Wrap(ship.Description());
	
	// Pad by 10 pixels on the top and bottom.
	descriptionHeight = description.Height() + 20;
}



void ShipInfoDisplay::UpdateAttributes(const Ship &ship)
{
	attributeLabels.clear();
	attributeValues.clear();
	attributesHeight = 10;
	
	const Outfit &attributes = ship.Attributes();
	
	// TODO: if in the millions, abbreviate, e.g. 1.45M
	attributeLabels.push_back(string());
	attributeValues.push_back(string());
	attributesHeight += 10;
	attributeLabels.push_back("cost:");
	attributeValues.push_back(Round(attributes.Cost()));
	attributesHeight += 20;
	
	attributeLabels.push_back(string());
	attributeValues.push_back(string());
	attributesHeight += 10;
	if(attributes.Get("shield generation"))
	{
		attributeLabels.push_back("shields charge / max:");
		attributeValues.push_back(Round(attributes.Get("shield generation"))
			+ " / " + Round(attributes.Get("shields")));
	}
	else
	{
		attributeLabels.push_back("shields:");
		attributeValues.push_back(Round(attributes.Get("shields")));
	}
	attributesHeight += 20;
	if(attributes.Get("hull repair rate"))
	{
		attributeLabels.push_back("hull repair / max:");
		attributeValues.push_back(Round(attributes.Get("hull repair rate"))
			+ " / " + Round(attributes.Get("hull")));
	}
	else
	{
		attributeLabels.push_back("hull:");
		attributeValues.push_back(Round(attributes.Get("hull")));
	}
	attributesHeight += 20;
	attributeLabels.push_back("crew / bunks:");
	attributeValues.push_back(Round(attributes.Get("required crew"))
		+ " / " + Round(attributes.Get("bunks")));
	attributesHeight += 20;
	attributeLabels.push_back("cargo space:");
	attributeValues.push_back(Round(attributes.Get("cargo space")));
	attributesHeight += 20;
	attributeLabels.push_back("fuel:");
	attributeValues.push_back(Round(attributes.Get("fuel capacity")));
	attributesHeight += 20;
	
	attributeLabels.push_back(string());
	attributeValues.push_back(string());
	attributesHeight += 10;
	attributeLabels.push_back("movement, full / no cargo:");
	attributeValues.push_back(string());
	attributesHeight += 20;
	double emptyMass = attributes.Get("mass");
	double fullMass = emptyMass + attributes.Get("cargo space");
	attributeLabels.push_back("max speed");
	attributeValues.push_back(Round(60. * attributes.Get("thrust") / attributes.Get("drag")));
	attributesHeight += 20;
	attributeLabels.push_back("acceleration");
	attributeValues.push_back(Round(60. * attributes.Get("thrust") / fullMass)
		+ " / " + Round(60. * attributes.Get("thrust") / emptyMass));
	attributesHeight += 20;
	attributeLabels.push_back("turning:");
	attributeValues.push_back(Round(60. * attributes.Get("turn") / fullMass)
		+ " / " + Round(60. * attributes.Get("turn") / emptyMass));
	attributesHeight += 20;
	
	// Find out how much outfit, engine, and weapon space the chassis has.
	map<string, double> chassis;
	static const string names[] = {
		"outfit space:", "outfit space",
		"    weapon capacity:", "weapon capacity",
		"    engine capacity:", "engine capacity",
		"guns:", "gun ports",
		"turrets:", "turrent mounts"
	};
	for(int i = 1; i < 10; i += 2)
		chassis[names[i]] = attributes.Get(names[i]);
	for(const auto &it : ship.Outfits())
		for(auto &cit : chassis)
			cit.second -= it.second * it.first->Get(cit.first);
	
	attributeLabels.push_back(string());
	attributeValues.push_back(string());
	attributesHeight += 10;
	for(int i = 0; i < 10; i += 2)
	{
		attributeLabels.push_back(names[i]);
		attributeValues.push_back(Round(attributes.Get(names[i + 1]))
			+ " / " + Round(chassis[names[i + 1]]));
		attributesHeight += 20;
	}
	
	tableLabels.clear();
	energyTable.clear();
	heatTable.clear();
	// Skip a spacer and the table header.
	attributesHeight += 30;
	
	tableLabels.push_back("idle:");
	energyTable.push_back(Round(60. * attributes.Get("energy generation")));
	heatTable.push_back(Round(60. * attributes.Get("heat generation")));
	attributesHeight += 20;
	tableLabels.push_back("moving:");
	energyTable.push_back(Round(
		-60. * (attributes.Get("thrusting energy") + attributes.Get("turning energy"))));
	heatTable.push_back(Round(
		60. * (attributes.Get("thrusting heat") + attributes.Get("turning heat"))));
	attributesHeight += 20;
	double firingEnergy = 0.;
	double firingHeat = 0.;
	for(const auto &it : ship.Outfits())
		if(it.first->IsWeapon())
		{
			firingEnergy += it.second * it.first->WeaponGet("firing energy")
				/ it.first->WeaponGet("reload");
			firingHeat += it.second * it.first->WeaponGet("firing heat")
				/ it.first->WeaponGet("reload");
		}
	tableLabels.push_back("firing:");
	energyTable.push_back(Round(-60. * firingEnergy));
	heatTable.push_back(Round(60. * firingHeat));
	attributesHeight += 20;
	tableLabels.push_back("max:");
	energyTable.push_back(Round(attributes.Get("energy capacity")));
	heatTable.push_back(Round(60. * emptyMass * .1));
	// Pad by 10 pixels on the top and bottom.
	attributesHeight += 30;
}



void ShipInfoDisplay::UpdateOutfits(const Ship &ship)
{
	outfitLabels.clear();
	outfitValues.clear();
	outfitsHeight = 0;
	int outfitsValue = 0;
	
	map<string, map<string, int>> listing;
	for(const auto &it : ship.Outfits())
	{
		listing[it.first->Category()][it.first->Name()] += it.second;
		outfitsValue += it.first->Cost() * it.second;
	}
	
	for(const auto &cit : listing)
	{
		// Pad by 10 pixels before each category.
		outfitLabels.push_back(string());
		outfitValues.push_back(string());
		outfitsHeight += 10;
		outfitLabels.push_back(cit.first + ':');
		outfitValues.push_back(string());
		outfitsHeight += 20;
		
		for(const auto &it : cit.second)
		{
			outfitLabels.push_back(it.first);
			outfitValues.push_back(to_string(it.second));
			outfitsHeight += 20;
		}
	}
	// Pad by 10 pixels on the top and bottom.
	outfitsHeight += 10;
	
	
	saleLabels.clear();
	saleValues.clear();
	saleHeight = 0;
	int totalValue = ship.Attributes().Cost();
	
	saleLabels.push_back(string());
	saleValues.push_back(string());
	saleHeight += 10;
	saleLabels.push_back("This ship will sell for:");
	saleValues.push_back(string());
	saleHeight += 20;
	saleLabels.push_back("empty hull:");
	saleValues.push_back(Round(totalValue - outfitsValue) + " credits");
	saleHeight += 20;
	saleLabels.push_back("  + outfits:");
	saleValues.push_back(Round(outfitsValue) + " credits");
	saleHeight += 20;
	saleLabels.push_back("= total:");
	saleValues.push_back(Round(totalValue) + " credits");
	saleHeight += 20;
	
	// Pad by 10 pixels on the top and bottom.
	saleHeight += 10;
}
