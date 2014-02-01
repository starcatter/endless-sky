/* Projectile.cpp
Michael Zahniser, 15 Jan 2014

Function definitions for the Projectile class.
*/

#include "Projectile.h"

#include "Mask.h"
#include "Outfit.h"
#include "Ship.h"
#include "Sprite.h"

#include <cmath>

using namespace std;



Projectile::Projectile(const Ship &parent, Point position, Angle angle, const Outfit *weapon)
	: weapon(weapon), animation(weapon->WeaponSprite()),
	system(parent.GetSystem()),
	position(position), velocity(parent.Velocity()), angle(angle),
	targetShip(parent.GetTargetShip()), government(parent.GetGovernment()),
	lifetime(weapon->WeaponGet("lifetime"))
{
	double inaccuracy = weapon->WeaponGet("inaccuracy");
	if(inaccuracy)
		this->angle += Angle::Random(inaccuracy) - Angle::Random(inaccuracy);
	
	velocity += this->angle.Unit() * weapon->WeaponGet("velocity");
}



Projectile::Projectile(const Projectile &parent, const Outfit *weapon)
	: weapon(weapon), animation(weapon->WeaponSprite()),
	system(parent.system),
	position(parent.position), velocity(parent.velocity), angle(parent.angle),
	targetShip(parent.targetShip), government(parent.government),
	lifetime(weapon->WeaponGet("lifetime"))
{
	double inaccuracy = weapon->WeaponGet("inaccuracy");
	if(inaccuracy)
		this->angle += Angle::Random(inaccuracy) - Angle::Random(inaccuracy);
	
	velocity += this->angle.Unit() * weapon->WeaponGet("velocity");
}



// This returns false if it is time to delete this projectile.
bool Projectile::Move(list<Effect> &effects)
{
	if(--lifetime <= 0)
	{
		if(lifetime > -100)
			for(const auto &it : weapon->DieEffects())
				for(int i = 0; i < it.second; ++i)
				{
					effects.push_back(*it.first);
					effects.back().Place(position, velocity, angle);
				}
		
		return false;
	}
	
	// If the target has left the system, stop following it.
	const Ship *target = Target();
	if(target && target->GetSystem() != system)
	{
		targetShip.reset();
		target = nullptr;
	}
	
	double turn = weapon->WeaponGet("turn");
	double accel = weapon->WeaponGet("acceleration");
	int homing = weapon->WeaponGet("homing");
	if(target && homing)
	{
		Point d = position - target->Position();
		double drag = weapon->WeaponGet("drag");
		double trueVelocity = drag ? accel / drag : velocity.Length();
		double stepsToReach = d.Length() / trueVelocity;
		bool isFacingAway = d.Dot(angle.Unit()) > 0.;
		
		// At the highest homing level, compensate for target motion.
		if(homing == 4)
		{
			// Adjust the target's position based on where it will be when we
			// reach it (assuming we're pointed right towards it).
			d -= stepsToReach * target->Velocity();
			stepsToReach = d.Length() / trueVelocity;
		}
		
		double cross = d.Unit().Cross(angle.Unit());
		
		// The very dumbest of homing missiles lose their target if pointed
		// away from it.
		if(isFacingAway && homing == 1)
			targetShip.reset();
		else
		{
			double desiredTurn = (180. / M_PI) * asin(cross);
			if(fabs(desiredTurn) > turn)
				turn = copysign(turn, desiredTurn);
			else
				turn = desiredTurn;
			
			// Levels 3 and 4 stop accelerating when facing away.
			if(homing >= 3)
			{
				double stepsToFace = desiredTurn / turn;
		
				// If you are facing away from the target, stop accelerating.
				if(stepsToFace * 1.5 > stepsToReach)
					accel = 0.;
			}
		}
	}
	// If a weapon is homing but has no target, do not turn it.
	else if(homing)
		turn = 0.;
	
	if(turn)
		angle += Angle(turn);
	
	if(accel)
	{
		velocity += accel * angle.Unit();
		velocity *= 1. - weapon->WeaponGet("drag");
	}
	
	position += velocity;
	
	return true;
}



// This is called when a projectile "dies," either of natural causes or
// because it hit its target.
void Projectile::MakeSubmunitions(std::list<Projectile> &projectiles) const
{
	for(const auto &it : weapon->Submunitions())
		for(int i = 0; i < it.second; ++i)
			projectiles.emplace_back(*this, it.first);
}



// Check if this projectile collides with the given step, with the animation
// frame for the given step.
double Projectile::CheckCollision(const Ship &ship, int step) const
{
	double radius = weapon->WeaponGet("trigger radius");
	if(radius > 0. && ship.GetSprite().GetMask(step).WithinRange(
			position - ship.Position(), angle, radius))
		return 0.;
	
	return ship.GetSprite().GetMask(step).Collide(
		position - ship.Position(), velocity, ship.Facing());
}



// Check if the given ship is within this projectile's blast radius. (The
// projectile will not explode unless it is also within the trigger radius.)
bool Projectile::InBlastRadius(const Ship &ship, int step) const
{
	double radius = weapon->WeaponGet("blast radius");
	if(radius <= 0.)
		return false;
	
	return ship.GetSprite().GetMask(step).WithinRange(
		position - ship.Position(), angle, radius);
}



// This projectile hit something. Create the explosion, if any. This also
// marks the projectile as needing deletion.
void Projectile::Explode(std::list<Effect> &effects, double intersection)
{
	for(const auto &it : weapon->HitEffects())
		for(int i = 0; i < it.second; ++i)
		{
			effects.push_back(*it.first);
			effects.back().Place(
				position + velocity * intersection, velocity, angle);
		}
	lifetime = -100;
}



// This projectile hit the given ship. Damage that ship.
void Projectile::Hit(Ship &ship) const
{
	ship.TakeDamage(
		weapon->WeaponGet("shield damage"),
		weapon->WeaponGet("hull damage"),
		velocity.Unit() * weapon->WeaponGet("hit force"));
}



// This projectile was killed, e.g. by an anti-missile system.
void Projectile::Kill()
{
	lifetime = 0;
}



// Find out if this is a missile, and if so, how strong it is (i.e. what
// chance an anti-missile shot has of destroying it).
int Projectile::MissileStrength() const
{
	return static_cast<int>(weapon->WeaponGet("missile strength"));
}



// Get the projectiles characteristics, for drawing.
const Animation &Projectile::GetSprite() const
{
	return animation;
}



const Point &Projectile::Position() const
{
	return position;
}



const Point &Projectile::Velocity() const
{
	return velocity;
}



const Angle &Projectile::Facing() const
{
	return angle;
}



// Get the facing unit vector times the scale factor.
Point Projectile::Unit() const
{
	return angle.Unit() * .5;
}


	
// Find out which ship this projectile is targeting.
const Ship *Projectile::Target() const
{
	auto ptr = targetShip.lock();
	return ptr.get();
}



// Find out which government this projectile belongs to.
const Government *Projectile::GetGovernment() const
{
	return government;
}
