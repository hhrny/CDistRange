operator cdist_range alias CDIST_RANGE pattern _ op [_, _, _, _]
operator cdrange alias CDRANGE pattern _ op [_, _, _, _]
operator rtreefilter alias RTREEFILTER pattern op (_, _, _, _, _)
operator tbtreefilter alias TBTREEFILTER pattern op (_, _, _, _, _)
operator setifilter alias SETIFILTER pattern op (_, _, _, _)
operator gridblupdaterelrtree alias GRIDBLUPDATERELRTREE pattern _ op [_, _]
operator blupdaterelrtree alias BLUPDATERELRTREE pattern _ op [_, _]
operator mergertree alias MERGERTREE pattern op ( _, _ )
operator streamgridblupdatertree alias STREAMGRIDBLUPDATERTREE pattern _ op [ _, _, _ ]
operator gridbulkloadrtree alias GRIDBULKLOADRTREE pattern _ op [ _, _ ]
operator streamblupdatertree alias STREAMBLUPDATERTREE pattern _ op [ _, _, _ ]
operator streamoboupdatertree alias STREAMOBOUPDATERTREE pattern _ op [ _, _, _ ]
operator streamupdatetbtree alias STREAMUPDATETBTREE pattern _ op [ _, _ ]
operator streamupdatebltbtree alias STREAMUPDATEBLTBTREE pattern _ op [ _, _ ]
