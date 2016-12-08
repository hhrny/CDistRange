#include <cstdlib>
#include <assert.h>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
using namespace std;

/*
 * 3D Point
 */

class Point3D
{
    public:
        static const int MaxVal = 1<<20, MinVal = -(1<<20);
        int x, y, t;
        unsigned long long toULL(){
            unsigned long long result = 0, tmp;
            if(! (t >= MinVal && t <= MaxVal)){
                cout<<"Error: value t out of range!"<<endl;
                return 0;
            }
            if(! (y >= MinVal && y <= MaxVal)){
                cout<<"Error: value y out of range!"<<endl;
                return 0;
            }
            if(! (x >= MinVal && x <= MaxVal)){
                cout<<"Error: value x out of range!"<<endl;
                return 0;
            }
            result = t + MaxVal;
            result = result << 21;
            tmp = y + MaxVal;
            result |= tmp;
            result = result << 21;
            tmp = x + MaxVal;
            result |= tmp;
            return result;
        }
        void ReadFromULL(unsigned long long val){
            unsigned long long tmp = (1 << 21) - 1;
            x = (int)(val & tmp) - MaxVal;
            val = val >> 21;
            y = (int)(val & tmp) - MaxVal;
            val = val >> 21;
            t = (int)(val & tmp) - MaxVal;
        }
        void Print(ostream &out){
            out<<"3D Point:(x:"<<x<<",\ty:"<<y<<",\tt:"<<t<<")";
        }
        double DistanceSquare(Point3D &p){
            return (p.x-x)*(p.x-x) + (p.y-y)*(p.y-y) + (p.t-t)*(p.t-t);
        }
        double Distance(Point3D &p){
            return sqrt(DistanceSquare(p));
        }
        bool operator==(Point3D &p) const{
            if(p.x == x && p.y == y && p.t == t){
                return true;
            }
            return false;
        }
        bool operator!=(Point3D &p) const{
            if(p.x == x && p.y == y && p.t == t){
                return false;
            }
            return true;
        }
};

template <unsigned dim>
class Rectangle
{
    public:
        int max[dim];
        int min[dim];

        static Rectangle<dim> getRandomRect(){
            int i;
            Rectangle<dim> result;
            for(i = 0; i < dim; i ++){
                result.min[i] = rand()%100000;
                result.max[i] = result.min[i] + rand()%(100000-result.min[i]);
            }
            return result;
        }
        
        void Print(ostream &out){
            int i;
            out<<"rect"<<dim<<":(";
            for(i = 0; i < dim; i ++){
                out<<"("<<min[i]<<","<<max[0]<<")";
            }
            out<<")"<<endl;
        }
        Rectangle<dim> Union(Rectangle<dim> &rect){
            int i;
            Rectangle<dim> result;
            for(i = 0; i < dim; i ++){
                this->min[i] = result.min[i] = this->min[i] < rect.min[i] ? this->min[i] : rect.min[i];
                this->max[i] = result.max[i] = this->max[i] > rect.max[i] ? this->max[i] : rect.max[i];
            }
            return result;
        }
        int MinD(int index){
            assert(index >= 0);
            assert(index < dim);
            return min[index];
        }
        int MaxD(int index){
            assert(index >= 0);
            assert(index < dim);
            return max[index];
        }
};

Point3D GetCenterofPoints(vector<Point3D> points){
    int sumx = 0, sumy = 0, sumt = 0;
    vector<Point3D>::iterator it;
    Point3D result;

    for(it = points.begin(); it != points.end(); it ++){
        sumx += it->x;
        sumy += it->y;
        sumt += it->t;
    }
    result.x = sumx/points.size();
    result.y = sumy/points.size();
    result.t = sumt/points.size();
    return result;
}

// k-means 
int KMeans(unsigned int k, vector<Point3D> &points, vector<vector<Point3D> > &result){
    unsigned int j = 0, i = 0, min;
    bool changed;
    double mindist, dist;
    Point3D tmppoint;
    vector<Point3D> center, tmp;
    vector<Point3D>::iterator it;
    tmp.clear();
    result.clear();
    // size < k
    if(points.size() < k){
        cout<<"error: the number of points is less than k value!"<<endl;
        return -1;
    }
    // size == k
    if(points.size() == k){
        for(i = 0; i < k; i ++){
            tmp.push_back(points[i]);
            result.push_back(tmp);
            tmp.clear();
        }
        return k;
    }
    // initialize k center
    for(i = 0; i < k; i ++){
        center.push_back(points[i]);
        result.push_back(tmp);
    }
    for(j = 0; j < 100; j ++){
        changed = false;
        for(it = points.begin(); it != points.end(); it++){
            // despatch all the points to k collection
            mindist = it->DistanceSquare(center[0]);
            min = 0;
            for(i = 1; i < k; i ++){
                // find the minimum distance center
                dist = it->DistanceSquare(center[i]);
                if(dist < mindist){
                    min = i;
                    mindist = dist;
                }
            }
            // push the point to minimum distance center
            result[min].push_back(*it);
        }
        for(i = 0; i < k; i ++){
            // compute the new center
            tmppoint = GetCenterofPoints(result[i]);
            if(tmppoint != center[i]){
                // update the center
                center[i] = tmppoint;
                changed = true;
            }
        }
        if(! changed){
            // end to k-means
            return k;
        }
        // clear the result
        for(i = 0; i < k; i ++){
            result[i].clear();
        }
    }
    cout<<"Warning: can not divide points to k collection!"<<endl;
    return -1;
}

class Entry
{
    public:
        int          id;
        Rectangle<3> box;

        Entry(){}
        Entry(Rectangle<3> &b, int i){
            box = b;
            id = i;
        }
};

class Node
{
    public:
        vector<Entry> entries;
        int           entrycount;
        
        Node(){
            entries.clear();
            entrycount = 0;
        }
        void Clear(){
            entries.clear();
            entrycount = 0;
        }
        int EntryCount(){
            return entrycount;
        }
        int Insert(Entry e){
            entries.push_back(e);
            entrycount ++;
        }
        Entry &operator[](int index){
            assert(index >= 0);
            assert(index < entrycount);
            return entries[index];
        }
        Rectangle<3> BoundingBox(){
            Rectangle<3> box;
            vector<Entry>::iterator it;
            assert(entries.size());
            it = entries.begin();
            box = it->box;
            it ++;
            for(; it != entries.end(); it++){
                box.Union(it->box);
            }
            return box;
        }
};

/*
   bulk load using grid

   class RTreeLevel : the one level of rtree node when bulk load using grid

*/
// 3 dimission, x, y and t
class RTreeLevel
{
    private:
        // the next level pointer, when the number of node entry in this level gird is bigger than max entries number
        // generate a next level, and insert the entries to next level
        RTreeLevel *nextlevel;
        // all the entries in this level
        int noentries;
        int entrycount, nodecount;
        // length, width and time of each grid unit 
        double  unitx, unity, unittime;
        // leaf
        bool isleaf;
        int maxentries, minentries;
        map<unsigned long long, Node *> grid;
        map<unsigned long long, Node *>::iterator it;
        
        //
        int Insert(Node *node);
    public:
        static int idcount;
        RTreeLevel(bool isleaf);
        RTreeLevel(bool isleaf, double ux, double uy, double ut);
        ~RTreeLevel();

        Point3D convertMBR2P(Rectangle<3> box){
            Point3D result;
            result.x = (box.MaxD(0)-box.MinD(0))/2/unitx;
            result.y = (box.MaxD(1)-box.MinD(1))/2/unity;
            result.t = (box.MaxD(2)-box.MinD(2))/2/unittime;
            return result;
        }
        int SaveNode2RTree(Node *node);
        //int Insert(TupleId tid, const Rectangle<3> &box);
        int Insert(int id, Rectangle<3> box);
        // get the root of all the node
        int GetRoot();
        int GetHeight();
        int Append(RTreeLevel *rl);
        void SetRTreeRoot();
        int EntryCount();
        int NodeCount();
};


RTreeLevel::RTreeLevel(bool isleaf){
    this->isleaf = isleaf;
    this->maxentries = 64;
    this->minentries = 32;
    this->unitx = 10.0;
    this->unity = 10.0;
    this->unittime = 10.0;
    this->nextlevel = NULL;
    entrycount = 0;
    nodecount = 0;
}

RTreeLevel::RTreeLevel(bool isleaf, double ux, double uy, double ut){
    this->isleaf = isleaf;
    this->maxentries = 64;
    this->minentries = 32;
    this->unitx = ux;
    this->unity = uy;
    this->unittime = ut;
    this->nextlevel = NULL;
    entrycount = 0;
    nodecount = 0;
}

RTreeLevel::~RTreeLevel(){
    if(nextlevel != NULL){
        delete nextlevel;
    }
    for(it = grid.begin(); it != grid.end(); it++){
        delete it->second;
    }
    grid.clear();
}

// save the data of node to rtree
// and insert the index entry of this node to nextlevel
// and minus the number of entries of this node
int RTreeLevel::SaveNode2RTree(Node *node){
    int AppendRecord = 0, tmp = 0;
    Rectangle<3> bbox;
    int  sid;

    if(node == NULL){
        cout<<"node is null!"<<endl;
        return -1;
    }
    bbox = node->BoundingBox();
    tmp = node->EntryCount();
    // save node to rtree
    //
    sid = RTreeLevel::idcount++;
    cout<<"save "<<tmp<<" entris to rtree, bounding box is ";
    bbox.Print(cout);
    if(nextlevel == NULL){
        nextlevel = new RTreeLevel(false, unitx, unity, unittime);
    }
    nextlevel->Insert(sid, bbox);
    // minus the save entries
    noentries -= tmp;
    return 1;
}

// insert a entry
int RTreeLevel::Insert(int id, Rectangle<3> box){
    Point3D     p;
    unsigned long long key;
    
    p = convertMBR2P(box);
    key = p.toULL();
    it = grid.find(key);
    if(it == grid.end()){
        // this is no node with key
        grid[key] = new Node();
    }
    // insert the entry to node
    grid[key]->Insert(Entry(box, id));
    noentries ++;
    entrycount ++;
    // make sure the node is no full
    if(grid[key]->EntryCount() >= maxentries){
        // node is full
        SaveNode2RTree(grid[key]);
        grid[key]->Clear();
        nodecount ++;
    }
    return 1;
}
// insert all entries of a node
int RTreeLevel::Insert(Node *node){
    int            i; 
    Point3D        p;
    unsigned long long key;

    if(node == NULL){
        cout<<"node is null!"<<endl;
        return -1;
    }
    if(node->EntryCount() <= 0){
        return 0;
    }
    p = convertMBR2P(node->BoundingBox());
    key = p.toULL();
    it = grid.find(key);
    if(it == grid.end()){
        // there is no node in p
        grid[key] = new Node();
    }
    // insert all the entry in node to new node
    for(i = 0; i < node->EntryCount(); i ++){
        // get a entry of this node
        // insert a entry to this node
        grid[key]->Insert((*node)[i]);
        noentries ++;
        entrycount ++;
        // make sure the node is no full
        if(grid[key]->EntryCount() >= maxentries){
            // node is full
            SaveNode2RTree(grid[key]);
            grid[key]->Clear();
            nodecount ++;
        }
    }
    return 1;
}

int RTreeLevel::Append(RTreeLevel *rl){
    if(rl->isleaf != isleaf){
        cout<<"error: in append rtree level!"<<endl;
        return -1;
    }
    cout<<"test append 1"<<endl;
    for(it = rl->grid.begin(); it != rl->grid.end(); it++){
        Insert(it->second);
    }
    cout<<"test append 2"<<endl;
    if(rl->nextlevel != NULL){
        nextlevel->Append(rl->nextlevel);
    }
    cout<<"test append 3"<<endl;
    return 1;
}

int RTreeLevel::GetHeight(){
    if(nextlevel == NULL){
        return 1;
    }
    return nextlevel->GetHeight() + 1;
}

int RTreeLevel::GetRoot(){
    int          AppendRecord, i, j;
    unsigned long long key;
    int  sid;
    Node *n;
    Point3D                p;
    vector<Point3D>        points;
    vector<Point3D>::iterator pit;
    vector<vector<Point3D> > result;
    RTreeLevel             *rl, *oldrl = this;
    while(oldrl->noentries > (maxentries * 2)){
        // more than two node to save
        rl = new RTreeLevel(isleaf, oldrl->unitx*2, oldrl->unity*2, oldrl->unittime*2);
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            rl->Insert(it->second);
        }
        if(rl->nextlevel != NULL){
            // the new rtreelevel's next level is no NULL
            // add all the entry in the new rtreelevel's next level to this rtreelevel
            if(this->nextlevel == NULL){
                // check the nextlevel whether is null
                this->nextlevel = new RTreeLevel(false, unitx, unity, unittime);
            }
            // problem is here
            this->nextlevel->Append(rl->nextlevel);
            cout<<"test delete 1"<<endl;
            delete rl->nextlevel;
            cout<<"test delete 2"<<endl;
            rl->nextlevel = NULL;
        }
        if(oldrl != this){
            delete oldrl;
        }
        cout<<"test get root 1"<<endl;
        oldrl = rl;
    }
    // old rtreelevel
    if(oldrl->noentries > maxentries){
        // devide all the entries in this level to two node
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
        cout<<"test get root 2"<<endl;
            p.ReadFromULL(it->first);
            points.push_back(p);
        }
        if(2 != KMeans(2, points, result)){
            cout<<"error in dispatch all entries to two node!"<<endl;
            return 0;
        }
        for(i = 0; i < 2; i ++){
            n = new Node();
            for(pit = result[i].begin(); pit != result[i].end(); pit++){
                key = pit->toULL();
                for(j = 0; j < (oldrl->grid[key])->EntryCount(); j++){
                    n->Insert((*(oldrl->grid[key]))[j]);
                    cout<<"test get root 3"<<endl;
                }
            }
            SaveNode2RTree(n);
            delete n;
        }
    }
    else if(oldrl->noentries >= minentries){
        // the left entries of this level is less than max entries, collect all the entries as a node
        n = new Node();
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            for(i = 0; i < (it->second)->EntryCount(); i++){
                n->Insert((*(it->second))[i]);
                cout<<"test get root 4"<<endl;
            }
        }
        SaveNode2RTree(n);
        delete n;
        if(oldrl != this){
            delete oldrl;
        }
    }
    else if(this->nextlevel == NULL){
        // this is a root level
        // collect all the entries in this level as a root node
        n = new Node();
        for(it = oldrl->grid.begin(); it != oldrl->grid.end(); it++){
            for(i = 0; i < (it->second)->EntryCount(); i++){
                n->Insert((*(it->second))[i]);
                cout<<"test get root 5"<<endl;
            }
        }
        // save root
        //
        cout<<"produce a root and save to file!"<<endl;
        sid = RTreeLevel::idcount++;
        delete n;
        if(oldrl != this){
            delete oldrl;
        }
        return sid;
    }
    else{
        cout<<"error in function GetRoot(): the number of entries is less than the min number of entry!"<<endl;
        return 0;
    }
    return nextlevel->GetRoot();
}

int RTreeLevel::EntryCount(){
    if(nextlevel == NULL){
        return entrycount;
    }
    return entrycount + nextlevel->EntryCount();
}

int RTreeLevel::NodeCount(){
    if(nextlevel == NULL){
        return nodecount;
    }
    return nodecount + nextlevel->NodeCount();
}

void RTreeLevel::SetRTreeRoot(){
    cout<<"Root:  "<<GetRoot()<<endl;;
    cout<<"Height:"<<GetHeight()<<endl;
    cout<<"entry count:"<<EntryCount()<<endl;
    cout<<"node count:"<<NodeCount()<<endl;
}

int RTreeLevel::idcount = 0;

int main(){
    int i;
    RTreeLevel *rl;
    rl = new RTreeLevel(true);
    for(i = 0; i < 100; i ++){
        rl->Insert(i, Rectangle<3>::getRandomRect());
    }
    rl->SetRTreeRoot();
    return 0;
}

