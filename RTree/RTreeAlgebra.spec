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

operator creatertree alias CREATERTREE pattern _ op [ _ ]
operator bulkloadrtree alias BULKLOADRTREE pattern _ op [ _ ]
operator windowintersects alias WINDOWINTERSECTS pattern _ _ op [ _ ]
operator windowintersectsS alias WINDOWINTERSECTSS pattern _ op [ _ ]
operator gettuples alias GETTUPLES pattern _ _ op
operator gettuples2 alias GETTUPLES2 pattern _ _ op[ _ ]
operator gettuplesdbl alias GETTUPLESDBL pattern _ _ op[ _ ]
operator nodes alias NODES pattern op ( _ )
operator entries alias ENTRIES pattern op ( _ )
operator treeheight alias TREEHEIGHT pattern op ( _ )
operator no_nodes alias NO_NODES pattern op ( _ )
operator no_entries alias NO_ENTRIES pattern op ( _ )
operator bbox alias BBOX pattern op ( _, _ )
operator getFileInfo alias GETFILEINFO pattern op ( _ )
operator updatebulkloadrtree alias UPDATEBULKLOADRTREE pattern _ _ op [ _]
operator getRootNode alias GETROOTNODE pattern op ( _ )
operator getNodeInfo alias GETNODEINFO pattern op ( _ , _ )
operator getNodeSons alias GETNODESONS pattern op ( _ , _ )
operator getLeafEntries alias GETLEAFENTRIES pattern op ( _ , _ )
operator cyclicbulkload alias CYCLICBULKLOAD pattern _  op [ _, _, _, _ ]
operator rtree2MBRstream alias RTREE2MBRSTREAM pattern op ( _ )

operator dspatialJoin alias DSPATIALJOIN pattern _ _ _ _ op[_]


