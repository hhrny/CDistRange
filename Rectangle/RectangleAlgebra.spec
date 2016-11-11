#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science,
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

operator rectangle2 alias RECTANGLE2 pattern op ( _ , _, _, _)
operator rectangle3 alias RECTANGLE3 pattern op ( _ , _, _, _, _, _)
operator rectangle4 alias RECTANGLE4 pattern op ( _ , _, _, _, _, _, _, _)
operator rectangle8 alias RECTANGLE8 pattern op ( _ , _, _, _, _, _, _, _, _)

operator translate alias TRANSLATE pattern  _ op [list]
operator union alias UNION pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator intersects alias INTERSECTS pattern _ infixop _
operator intersection alias INTERSECTION pattern op ( _ , _)
operator distance alias DISTANCE pattern op ( _ , _ )
operator rectproject alias RECTPROJECT pattern op ( _ , _ , _ )
operator minD alias MIND pattern op ( _ , _ )
operator maxD alias MAXD pattern op ( _ , _ )
operator bbox alias BBOX pattern op( _, _ )
operator enlargeRect alias ENLARGERECT pattern op( _ )
operator scalerect alias SCALERECT pattern op( _, _, _ )
operator size alias SIZE pattern op( _ )
operator bboxintersects alias BBOXINTERSECTS pattern _ infixop _
operator cellnumber alias CELLNUMBER pattern op(_, _, _, _, _, _)
operator gridintersects alias GRIDINTERSECTS pattern op (_, _, _, _, _, _, _, _)
operator gridcell2rect alias GRIDCELL2RECT pattern op (_, _, _, _, _, _)
operator center alias CENTER pattern op ( _ )
operator partitionRect alias PARTITIONRECT pattern op (_,_,_)

