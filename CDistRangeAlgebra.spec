# operator cdist_range: 
#     (stream) cdist_range [(trip of query), (d1), (d2), (attribute name of trip in stream tuple)]
operator cdist_range alias CDIST_RANGE pattern _ op [_, _, _, _]
# operator rtreefilter:
#     (relation) rtreefilter [(rtree index), (boundingbox of query trip), (d1), (d2)]
operator rtreefilter alias RTREEFILTER pattern _ op [_, _, _, _]
# operator tbtreefilter
#     (relation) tbtreefilter [(tbtree index), (boundingbox of query trip), (d1), (d2)]
operator tbtreefilter alias TBTREEFILTER pattern _ op [_, _, _, _]
# operator setifilter
#     (seti) setifilter [(boundingbox of query trip), (d1), (d2)]
operator setifilter alias SETIFILTER pattern _ op [_, _, _]
# value of distance: 0.0 <= d1 <= inf or INF, 0.0 < d2 <= inf or INF
# inf or INF means positive infinity

# operator to update relation and generate rtree
operator updaterelrtree alias UPDATERELRTREE pattern _ op [_, _]
