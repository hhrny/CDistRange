operator loaddata alias LOADDATA pattern op ( _ )
operator loaddatafromdir alias LOADDATAFROMDIR pattern op ( _ )
operator loaduploadunit alias LOADUPLOADUNIT pattern op ( _ )
operator trajectorysplit alias TRAJECTORYSPLIT pattern _ op [_, _]
operator sizetest1 alias SIZETEST1 pattern op ( _ )
operator sizetest2 alias SIZETEST2 pattern op ( _ )
operator convertUU2UP alias CONVERTUU2UP pattern _ op [ _ ]
operator convertUP2MP alias CONVERTUP2MP pattern _ op [ _, _ ]
operator convertP2UU alias CONVERTP2UU pattern _ op [ _ ]
operator meanfilter alias MEANFILTER pattern _ op [ _ , _ ]
operator medianfilter alias MEDIANFILTER pattern _ op [ _ , _ ]
operator gkproject alias GKPROJECT pattern op ( _ )
operator pointminus alias POINTMINUS pattern _ infixop _
