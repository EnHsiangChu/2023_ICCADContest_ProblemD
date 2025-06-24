#include <iostream>
#include "structure.h"
using namespace std;

Tile* GoToPoint(Tile* tp, Point p)
{
	if (p.p_y < BOTTOM(tp))
	{
		do
		{
			tp = LB(tp);
		} while (p.p_y < BOTTOM(tp));
	}
	else
	{
		while (p.p_y >= TOP(tp))
			tp = RT(tp);
	}

	if (p.p_x < LEFT(tp))
	{
		do
		{
			do
			{
				tp = BLeft(tp);
			} while (p.p_x < LEFT(tp));
			
			if (p.p_y < TOP(tp))
				break;

			do
			{
				tp = RT(tp);
			} while (p.p_y >= TOP(tp));

		} while (p.p_x < LEFT(tp));
	}
	else
	{
		while (p.p_x >= RIGHT(tp))
		{
			do
			{
				tp = TRight(tp);
			} while (p.p_x >= RIGHT(tp));

			if (p.p_y >= BOTTOM(tp))
				break;

			do
			{
				tp = LB(tp);
			} while (p.p_y < BOTTOM(tp));
		}
	}
	return tp;
}

Tile* TiSrPoint(Tile* hintTile, Plane* plane, Point point) {
	Tile* tp;

	if (hintTile == NULL) {//Pointer to tile at which to begin search.
		tp = plane->pl_hint;
	}		
	else {
		tp = hintTile;
	}
		

	tp = GoToPoint(tp, point);
	plane->pl_hint = tp;
	
	return tp;
}

