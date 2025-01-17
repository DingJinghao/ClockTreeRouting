#include "Router.h"
#include "Setting.h"

using std::cout;
using std::endl;
using std::setprecision;

#define COMPLETE_LINKAGE 0
#define SINGLE_LINKAGE 1
#define L1 1
#define L2 2
#define eps 1e-6
const string padding(30, '=');

void TreeTopology::init(int leafNum, int sz) {
    leafNumber = leafNum;
    size = sz;
    cout << "Initialize topo: " << leafNum << " leaves and " << sz << " nodes in total" << endl;
    constructTree(true);
}
void TreeTopology::constructTree(bool modifyCurrentTree) {
    vector<vector<int>> DAG(size);
    // cout << "rows " << HC_result.rows() << endl;
    int n_merges = HC_result.rows();
    int cur_internal_node_idx = leafNumber;
    for (int i = 0; i < n_merges; i++) {
        int p1 = HC_result[i][0];
        int p2 = HC_result[i][1];
        DAG[cur_internal_node_idx].push_back(p1);
        DAG[cur_internal_node_idx].push_back(p2);
        cur_internal_node_idx++;
        // cout << "Merge " << p1 << " and " << p2 << endl;
    }
    // Construct tree
    tmp_root = make_shared<TreeNode>(size - 1);  // last merge point as root
    std::function<void(shared_ptr<TreeNode>)> buildTree = [&](shared_ptr<TreeNode> curNode) {
        int curId = curNode->id;
        id_treeNode[curId] = curNode;
        if (DAG[curId].size() != 0) {
            auto lc = make_shared<TreeNode>(DAG[curId][0]);
            auto rc = make_shared<TreeNode>(DAG[curId][1]);
            curNode->set_lc(lc);
            curNode->set_rc(rc);
            lc->set_par(curNode);
            rc->set_par(curNode);
            buildTree(curNode->lc);
            buildTree(curNode->rc);
            return;
        }
    };
    buildTree(tmp_root);

    std::function<void(shared_ptr<TreeNode>)> postOrderTraversal = [&](shared_ptr<TreeNode> curNode) {
        int curId = curNode->id;
        if (curNode->lc != NULL && curNode->rc != NULL) {
            postOrderTraversal(curNode->lc);
            postOrderTraversal(curNode->rc);
            cout << "Vis: " << curId << endl;
            return;
        } else {
            cout << "Vis: " << curId << endl;
        }
    };
    // postOrderTraversal(tmp_root);

    if (modifyCurrentTree == true) {
        root = tmp_root;
    }
}

void Router::init() {
    // read input and build data structure
    readInput();
}
void Router::readInput() {
    ifstream fin(setting.input_file_name);
    if (fin.fail()) {
        cout << "Fail to open file:" << setting.input_file_name << endl;
        exit(1);
    } else {
        cout << padding << "Successfully open input:" << setting.input_file_name << padding << endl;
    }
    string line;
/*    
0 0 5000000 5000000
source 0 0 0 0
num sink 4
1 1200000 1300000 35
2 4000000 1200000 35
3 1300000 3800000 35
4 3700000 3300000 35
*/
    /*
    while (getline(fin, line)) {
        istringstream iss(line);
        if (iss.str().find("MAX_RUNTIME") != std::string::npos) {
            MAX_RUNTIME = stoi(line.substr(line.find(" ") + 1));
            cout << "MAX_RUNTIME: " << MAX_RUNTIME << endl;
        }

        if (iss.str().find("END TAPS") != std::string::npos) {
            break;
        } else if (iss.str().find("TAPS") != std::string::npos) {
            NUM_TAPS = stoi(line.substr(line.find(" ") + 1));
            cout << "NUM_TAPS: " << NUM_TAPS << endl;
        } else if (iss.str().find("TAP") != std::string::npos) {
            char buf[10];
            int tap_id;
            double x, y;
            iss >> buf >> tap_id >> x >> y;
            taps.emplace_back(tap_id, x, y);
        }
    } 
    */
   while (getline(fin, line)) {
        istringstream iss(line);
        // if (iss.str().find("MAX_RUNTIME") != std::string::npos) {
        //     MAX_RUNTIME = stoi(line.substr(line.find(" ") + 1));
        //     cout << "MAX_RUNTIME: " << MAX_RUNTIME << endl;
        // }
        // if (iss.str().find("END TAPS") != std::string::npos) {
        //     break;
        if (iss.str().find("sink") != std::string::npos) {
            cout << "==================" << line.substr(line.find(" ") + 1) << "==================" << endl;
            NUM_TAPS = stoi(line.substr(line.rfind(" ") + 1));
            cout << "NUM_TAPS: " << NUM_TAPS << endl;
            for(int i = 0; i < NUM_TAPS; i++) {
                getline(fin, line);
                istringstream iss_2(line);
                int tap_id; 
                double x, y, z;
                iss_2 >> tap_id >> x >> y >> z;
                taps.emplace_back(tap_id, x, y);
            }
            break;
        }
    }
    for (auto& tap : taps) {
        cout << tap << endl;
    }
    cout << padding << "Finish Reading input" << padding << endl;
}

// Recursive Geometric Matching
void Router::RGM() {
    // TODO
}

// nearest neighbor topology
void Router::NS() {
    // TODO
}

// Agglomerative  Hierarchical Clustering
void Router::HC() {
    // 1. Create XY array
    // vector<vector<double>> points;
    string points_str;
    points_str += "[";
    for (int i = 0; i < taps.size(); i++) {
        points_str += taps[i].str_xy();
        if (i != taps.size() - 1) {
            points_str += ",";
        }
    }
    points_str += "]";
    // cout <<"Points: " << points_str << endl;

    // 2. Create clusterizer
    using namespace alglib;
    clusterizerstate s;
    ahcreport rep;
    real_2d_array xy(points_str.c_str());
    // int linkage_type = SINGLE_LINKAGE;
    int linkage_type = COMPLETE_LINKAGE;
    int metric = L1;

    clusterizercreate(s);
    clusterizersetpoints(s, xy, metric);  // manhattan dist
    clusterizersetahcalgo(s, linkage_type);
    clusterizerrunahc(s, rep);
    printf("%s\n", rep.z.tostring().c_str());

    // 3. Construct binary tree topology
    topo = make_shared<TreeTopology>(rep.z);
    topo->init(xy.rows(), 2 * xy.rows() - 1);

    cout << padding << "Finish Topology generation" << padding << endl;
}

inline double L1Dist(GridPoint p1, GridPoint p2) { return abs(p1.x - p2.x) + abs(p1.y - p2.y); }

Segment TRRintersect(TRR& trr1, TRR& trr2) {
    // get four edges
    // cout << "Merging: " << trr1 << " and " << trr2 << endl;
    vector<GridPoint> trr1_boundary_grid;
    vector<GridPoint> trr2_boundary_grid;
    vector<Segment> trr1_Sides;
    vector<Segment> trr2_Sides;
    if (trr1.core.slope() > 0) {
        trr1_boundary_grid.emplace_back(trr1.core.p1.x, trr1.core.p1.y - trr1.radius);
        trr1_boundary_grid.emplace_back(trr1.core.p2.x + trr1.radius, trr1.core.p2.y);
        trr1_boundary_grid.emplace_back(trr1.core.p2.x, trr1.core.p2.y + trr1.radius);
        trr1_boundary_grid.emplace_back(trr1.core.p1.x - trr1.radius, trr1.core.p1.y);  // clock-wise
    } else if (trr1.core.slope() < 0) {
        trr1_boundary_grid.emplace_back(trr1.core.p1.x + trr1.radius, trr1.core.p1.y);
        trr1_boundary_grid.emplace_back(trr1.core.p2.x, trr1.core.p2.y + trr1.radius);
        trr1_boundary_grid.emplace_back(trr1.core.p2.x - trr1.radius, trr1.core.p2.y);
        trr1_boundary_grid.emplace_back(trr1.core.p1.x, trr1.core.p1.y - trr1.radius);  // clock-wise
    } else {                                                                            // leaf node
        trr1_boundary_grid.emplace_back(trr1.core.p1.x, trr1.core.p1.y - trr1.radius);
        trr1_boundary_grid.emplace_back(trr1.core.p1.x + trr1.radius, trr1.core.p1.y);
        trr1_boundary_grid.emplace_back(trr1.core.p1.x, trr1.core.p1.y + trr1.radius);
        trr1_boundary_grid.emplace_back(trr1.core.p1.x - trr1.radius, trr1.core.p1.y);  // clock-wise
    }

    if (trr2.core.slope() > 0) {
        trr2_boundary_grid.emplace_back(trr2.core.p1.x, trr2.core.p1.y - trr2.radius);
        trr2_boundary_grid.emplace_back(trr2.core.p2.x + trr2.radius, trr2.core.p2.y);
        trr2_boundary_grid.emplace_back(trr2.core.p2.x, trr2.core.p2.y + trr2.radius);
        trr2_boundary_grid.emplace_back(trr2.core.p1.x - trr2.radius, trr2.core.p1.y);  // clock-wise
    } else if (trr2.core.slope() < 0) {
        trr2_boundary_grid.emplace_back(trr2.core.p1.x + trr2.radius, trr2.core.p1.y);
        trr2_boundary_grid.emplace_back(trr2.core.p2.x, trr2.core.p2.y + trr2.radius);
        trr2_boundary_grid.emplace_back(trr2.core.p2.x - trr2.radius, trr2.core.p2.y);
        trr2_boundary_grid.emplace_back(trr2.core.p1.x, trr2.core.p1.y - trr2.radius);  // clock-wise
    } else {                                                                            // leaf node
        trr2_boundary_grid.emplace_back(trr2.core.p1.x, trr2.core.p1.y - trr2.radius);
        trr2_boundary_grid.emplace_back(trr2.core.p1.x + trr2.radius, trr2.core.p1.y);
        trr2_boundary_grid.emplace_back(trr2.core.p1.x, trr2.core.p1.y + trr2.radius);
        trr2_boundary_grid.emplace_back(trr2.core.p1.x - trr2.radius, trr2.core.p1.y);  // clock-wise
    }

    for (int i = 0; i < 3; i++) {
        trr1_Sides.emplace_back(trr1_boundary_grid[i], trr1_boundary_grid[i + 1]);
        trr2_Sides.emplace_back(trr2_boundary_grid[i], trr2_boundary_grid[i + 1]);
    }
    trr1_Sides.emplace_back(trr1_boundary_grid[3], trr1_boundary_grid[0]);
    trr2_Sides.emplace_back(trr2_boundary_grid[3], trr2_boundary_grid[0]);

    // cout << "Print trr1's sides" << endl;
    // for (auto& seg1 : trr1_Sides) {
    //     cout << seg1 << endl;
    // }

    // cout << "Print trr2's sides" << endl;

    // for (auto& seg2 : trr2_Sides) {
    //     cout << seg2 << endl;
    // }

    // for 4*4 check intersect
    for (auto& seg1 : trr1_Sides) {
        for (auto& seg2 : trr2_Sides) {
            Segment seg = seg1.intersect(seg2);
            if (seg.id == 0) {
                return seg;
            }
        }
    }
    cout << "Cannot find intersection between two TRRs" << endl;
    Segment ret;
    ret.id = -1;
    return ret;
}
// Deferred-Merge Embedding
void Router::DME() {
    // Segment seg1(GridPoint(1.0,3.0),GridPoint(2.0,4.0));

    // Segment seg2(GridPoint(-0.5,6.5),GridPoint(6,0));

    // cout << seg1.intersect(seg2) << endl;
    // exit(1);

    vertexMS.resize(topo->size);
    vertexTRR.resize(topo->size);
    vertexDistE.resize(topo->size);

    // 1. Build Tree of Segments (bottom up)
    std::function<void(shared_ptr<TreeNode>)> postOrderTraversal = [&](shared_ptr<TreeNode> curNode) {
        int curId = curNode->id;
        if (curNode->lc != NULL && curNode->rc != NULL) {
            postOrderTraversal(curNode->lc);
            postOrderTraversal(curNode->rc);

            // create merging segment for curNode
            auto& ms_a = vertexMS[curNode->lc->id];
            auto& ms_b = vertexMS[curNode->rc->id];
            // get |e_a|, |e_b|
            double d = min(L1Dist(ms_a.p1, ms_b.p1), L1Dist(ms_a.p1, ms_b.p2));
            d = min(d, L1Dist(ms_a.p2, ms_b.p1));
            d = min(d, L1Dist(ms_a.p2, ms_b.p2));  // but why need to calc 2*2 possiblity?
            double e_a_dist = (ms_b.delay - ms_a.delay + d) / 2;
            double e_b_dist = (ms_a.delay - ms_b.delay + d) / 2;
            if (e_a_dist < 0 || e_b_dist < 0) {
                cout << "Skew too large" << endl;
                exit(1);
            }
            vertexDistE[curNode->lc->id] = e_a_dist;
            vertexDistE[curNode->rc->id] = e_b_dist;

            // get trr_a, trr_b
            TRR trr_a(ms_a, e_a_dist);
            TRR trr_b(ms_b, e_b_dist);
            vertexTRR[curNode->lc->id] = trr_a;
            vertexTRR[curNode->rc->id] = trr_b;
            // intersect trr_a, trr_b to get ms_v
            Segment ms_v = TRRintersect(trr_a, trr_b);
            // cout << "Merging result: " << ms_v << endl;
            if (ms_v.id == -1) {
                cout << "Merge failure" << endl;
                exit(1);
            }
            ms_v.delay = e_a_dist + ms_a.delay;
            vertexMS[curId] = ms_v;
            // cout << "Delay diff " << e_a_dist + ms_a.delay - (e_b_dist + ms_b.delay) << endl;
        } else {
            // Create ms for leaf node
            vertexMS[curId] = Segment(taps[curId], taps[curId]);
        }
    };
    postOrderTraversal(topo->root);
    cout  << "Finish bottom-up process"  << endl;

    // 2. Find Exact Placement(top down)
    pl.resize(topo->size);
    sol.resize(topo->size);
    auto& rootMS = vertexMS[topo->root->id];

    std::function<void(shared_ptr<TreeNode>)> preOrderTraversal = [&](shared_ptr<TreeNode> curNode) {
        int curId = curNode->id;

        if (curNode->lc != NULL && curNode->rc != NULL) {
            // handle curNode
            if (curNode == topo->root) {
                GridPoint tmp;
                // tmp.x = (rootMS.p1.x + rootMS.p2.x) /2;
                // tmp.y = (rootMS.p1.y + rootMS.p2.y) /2;
                clockSource = rootMS.p1;
                pl[curId] = rootMS.p1;

                //  clockSource = tmp;
                // pl[curId] = tmp;
            } else {
                auto& par = curNode->par;
                int parId = par->id;
                auto& trr_par = vertexTRR[parId];
                trr_par.core = Segment(pl[parId], pl[parId]);
                trr_par.radius = vertexDistE[curId];

                // cout <<std::fixed<< "Before merge: the value for trr_par is" << setprecision(2) << trr_par << endl;
                // if(trr_par.radius == 122663.50){
                //     cout << 3 << endl;
                // }
                Segment merged = trr_par.intersect(vertexMS[curId]);
                // if(merged.isLeaf() == false){
                //     cout << trr_par << " intersecting "<< vertexMS[curId] <<  endl;
                //     cout << " Not leaf" <<endl;
                //     cout << merged << endl;
                // }
                if (merged.id == -1) {
                    cout << "TRR-MS merging failed" << endl;
                    exit(1);
                }
                pl[curId] = merged.p1;
            }

            cout << "Steiner Point " << curId << " located at " << pl[curId] << endl;
            preOrderTraversal(curNode->lc);
            preOrderTraversal(curNode->rc);
        } else {
            // sinks
            pl[curId] = vertexMS[curId].p1;
            return;
        }
    };
    preOrderTraversal(topo->root);
    cout  << "Finish top-down process"  << endl;

    cout << padding << "Finished DME" << padding << endl;
}

void Router::route() {
    HC();  // try hierarchical clustering
    DME();
}
bool db_equal(double a, double b) { return abs(a - b) < eps; }
void Router::buildSolution() {
    // preorder traversal to buil grsteiner structure
    std::function<void(shared_ptr<TreeNode>)> preOrderTraversal = [&](shared_ptr<TreeNode> curNode) {
        int curId = curNode->id;
        if (curNode->lc != NULL && curNode->rc != NULL) {
            // handle curNode
            shared_ptr<GrSteiner>& curSteiner = sol[curId];
            auto& lc = curNode->lc;
            auto& rc = curNode->rc;
            shared_ptr<GrSteiner> lcSteiner = make_shared<GrSteiner>(pl[lc->id]);
            shared_ptr<GrSteiner> rcSteiner = make_shared<GrSteiner>(pl[rc->id]);

            // Connect lc
            if (db_equal(curSteiner->x, lcSteiner->x) || db_equal(curSteiner->y, lcSteiner->y)) {
                lcSteiner->set_par(curSteiner);
            } else {  // Use L-shape
                shared_ptr<GrSteiner> middle = make_shared<GrSteiner>(GridPoint(curSteiner->x, lcSteiner->y));
                lcSteiner->set_par(middle);
                middle->set_par(curSteiner);
            }
            if (db_equal(curSteiner->x, rcSteiner->x) || db_equal(curSteiner->y, rcSteiner->y)) {
                rcSteiner->set_par(curSteiner);
            } else {  // Use L-shape
                shared_ptr<GrSteiner> middle = make_shared<GrSteiner>(GridPoint(curSteiner->x, rcSteiner->y));
                rcSteiner->set_par(middle);
                middle->set_par(curSteiner);
            }
            sol[lc->id] = lcSteiner;
            sol[rc->id] = rcSteiner;
            preOrderTraversal(lc);
            preOrderTraversal(rc);
        } else {
            // sinks
            // pl[curId] = vertexMS[curId].p1;
            return;
        }
    };
    sol[topo->root->id] = make_shared<GrSteiner>(pl[topo->root->id]);
    preOrderTraversal(topo->root);
}


// void Router::reportTotalWL() {


//     std::function<void(shared_ptr<GrSteiner>, double&)> preOrderTraversal = [&](shared_ptr<GrSteiner> curNode,double& wl) {

//         if (curNode->lc != NULL && curNode->rc != NULL) {
        
//         }
//         if (curNode->par == NULL) {  // reached source
//             return;
//         }
//         auto &nxtNode = curNode->par;
//         wl += L1Dist(*curNode,*nxtNode);
//         preOrderTraversal(nxtNode, wl);
//     };
//     double total_wl = 0;
    
    
//     // check wirelength
//     cout << padding << "Finish Write Result" << padding << endl;
// }


void Router::writeSolution() {
    ofstream fout(setting.output_file_name);
    if (fout.fail()) {
        cout << "Fail to open file:" << setting.output_file_name << endl;
        exit(1);
    } else {
        cout << padding << "Successfully open input:" << setting.output_file_name << padding << endl;
    }
    double total_wl = 0;
    std::unordered_set< PointPair, PointPairHasher> calculated_edges;

    std::function<void(shared_ptr<GrSteiner>, double& wl)> traceToSource = [&](shared_ptr<GrSteiner> curNode,
                                                                               double& wl) {
        fout << *curNode << " ";
        if (curNode->par == NULL) {  // reached source
            return;
        }
        auto &nxtNode = curNode->par;
        wl += L1Dist(*curNode,*nxtNode);
        PointPair tmp(curNode->x,curNode->y,nxtNode->x,nxtNode->y);
        if(calculated_edges.find(tmp) == calculated_edges.end()){
            calculated_edges.insert(tmp);
            total_wl += L1Dist(*curNode,*nxtNode);
        }
        traceToSource(nxtNode, wl);
    };

    
    vector<double> wirelenghs(taps.size(), 0);
    fout << std::fixed << setprecision(2) << clockSource << endl;
    for (int tapId = 0; tapId < taps.size(); tapId++) {
        fout << tapId << " ";
        traceToSource(sol[tapId], wirelenghs[tapId]);
        cout << std::fixed << setprecision(2) << "WL for tap" << tapId << ": " << wirelenghs[tapId] << endl;
        fout << endl;
    }
    // check wirelength
    cout << "Total Wirelength: " << total_wl << endl;
    cout << padding << "Finish Write Result" << padding << endl;
}
