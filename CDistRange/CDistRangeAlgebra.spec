operator cdist_range alias CDIST_RANGE pattern _ op [_, _, _, _]
operator cdrange alias CDRANGE pattern _ op [_, _, _, _]
operator rtreefilter alias RTREEFILTER pattern op (_, _, _, _, _)
operator tbtreefilter alias TBTREEFILTER pattern op (_, _, _, _, _)
operator setifilter alias SETIFILTER pattern op (_, _, _, _)
operator updaterelrtree alias UPDATERELRTREE pattern _ op [_, _]
operator mergertree alias MERGERTREE pattern op ( _, _ )
operator streamupdatertree alias STREAMUPDATERTREE pattern _ op [ _, _, _ ]
operator streamupdatetbtree alias STREAMUPDATETBTREE pattern _ op [ _, _ ]
operator streamupdatebltbtree alias STREAMUPDATEBLTBTREE pattern _ op [ _, _ ]
