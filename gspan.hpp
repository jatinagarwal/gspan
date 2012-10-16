#ifndef GSPAN_H_
#define GSPAN_H_

#include <cassert>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <iostream>

#include <boost/graph/graph_traits.hpp>
#include <boost/foreach.hpp>

#define BR asm("int $3;")

namespace graph_alg
{
    // =============================================================================
    // template parameter GraphOps
    // -----------------------------------------------------------------------------
    //
    // ----------------------------
    // Graph types
    // ----------------------------
    //  graph_t
    //  vertex_index_t
    //  edge_index_t
    //
    // ----------------------------
    // Graph operations
    // ----------------------------
    //  graph_t* create_graph(const DFSCode& dfsc);
    //  vertex_index_t void_vertex_index();
    //  edge_index_t   void_edge_index();
    //  vertex_index_t    get_vertex_index(vertex_descriptor vd, const graph_t& g);
    //  vertex_descriptor get_vertex_descriptor(vertex_index vi, const graph_t& g);
    //  edge_index_t      get_edge_index(edge_descriptor ed, const graph_t& g);
    //  edge_descriptor   get_edge_descriptor(edge_index ei, const graph_t& g);
    //
    // ----------------------------
    // Label types:
    // ----------------------------
    //  vertex_label_t
    //  edge_label_t;
    //  vertex_label_ref_t
    //  edge_label_ref_t
    //
    // ----------------------------
    // Label operations:
    // ----------------------------
    //  vertex_label_ref_t vlabel (vertex_descriptor vd, const graph_t& g);
    //  vertex_label_ref_t vilabel(vertex_index_t vi, const graph_t& g);
    //  edge_label_ref_t   elabel (edge_descriptor ed, const graph_t& g);
    //  edge_label_ref_t   eilabel(edge_index_t ei, const graph_t& g);
    //  bool vlabel_equal         (vertex_label_ref_t, vertex_label_ref_t);
    //  bool vlabel_less          (vertex_label_ref_t, vertex_label_ref_t);
    //  bool vlabel_less_or_equal (vertex_label_ref_t, vertex_label_ref_t);
    //  bool elabel_equal         (edge_label_ref_t, edge_label_ref_t);
    //  bool elabel_less          (edge_label_ref_t, edge_label_ref_t);
    //  bool elabel_less_or_equal (edge_label_ref_t, edge_label_ref_t);
    //  bool               void_vlabel(vertex_label_ref_t);
    //  vertex_label_ref_t void_vlabel();
    //
    // =============================================================================


    // *****************************************************************************
    //                          Edge
    // *****************************************************************************
    template<class VI_, class EI_>
    struct Edge_
    {
	typedef VI_ VI;
	typedef EI_ EI;	
	VI vi_from, vi_to;
	EI ei;
    };

    template<class VI, class EI>
    inline std::ostream& operator<<(std::ostream& out, const Edge_<VI,EI>& e)
    {
	return out<<"("<<e.vi_from<<","<<e.vi_to<<")";
    }


    // *****************************************************************************
    //                          EdgeCode
    // *****************************************************************************
    template<class VI_, class EI_, class VL, class EL, class VLR, class ELR>
    struct EdgeCode_
    {
	typedef VI_ VI;
	typedef EI_ EI;
	VI vi_from, vi_to;
	VL vl_from, vl_to;
	EL el;
	EdgeCode_(VI vifrom, VI vito, VLR vlfrom, ELR el, VLR vlto)
	    :vi_from(vifrom), vi_to(vito), vl_from(vlfrom), vl_to(vlto), el(el) {}

	bool is_forward() const { return vi_from < vi_to; }
	operator std::pair<VI,VI> () const { return std::pair<VI,VI>(vi_from,vi_to); }
    };

    template<class EdgeCode, class GraphOps>
    bool edgecode_equal(const EdgeCode& ec1, const EdgeCode& ec2, const GraphOps& ops)
    {
	return
	    ec1.vi_from == ec2.vi_from && ec1.vi_to == ec2.vi_to &&
	    ops.vlabel_equal(ec1.vl_from, ec2.vl_from) &&
	    ops.vlabel_equal(ec1.vl_to, ec2.vl_to) &&
	    ops.elabel_equal(ec1.el, ec2.el);
    }

    template<class VI, class EI, class VL, class EL, class VLR, class ELR>
    inline std::ostream& operator<<(std::ostream& out, const EdgeCode_<VI,EI,VL,EL,VLR,ELR>& ec)
    {
	return out<<"("<<ec.vi_from<<","<<ec.vi_to<<", "
		  <<ec.vl_from<<","<<ec.el<<","<<ec.vl_to<<")";
    }

    // *****************************************************************************
    //                          DFSCode
    // *****************************************************************************
    template<class EdgeCode>
    class DFSCode_
    {
	std::vector<EdgeCode> dfsc_;
    public:
	const EdgeCode& operator[] (int i) const { return dfsc_[i]; }
	bool empty() const { return dfsc_.empty(); }
	unsigned int size() const { return dfsc_.size(); }
	typedef typename std::vector<EdgeCode>::const_iterator const_iterator;
	const_iterator begin() const { return dfsc_.begin(); }
	const_iterator end() const { return dfsc_.end(); }
	typename EdgeCode::VI max_vertex() const;
	void push(const EdgeCode& ec) { dfsc_.push_back(ec); }
	void pop() { dfsc_.pop_back(); }
    };


    template<class EdgeCode>
    typename EdgeCode::VI DFSCode_<EdgeCode>::max_vertex() const
    {
	typename EdgeCode::VI m = 0;
	for (const_iterator i = begin(); i != end(); ++i)
	    m = std::max(m, std::max(i->vi_from, i->vi_to));
	return m;
    }


    template<class EdgeCode>
    std::ostream& operator<<(std::ostream& out, const DFSCode_<EdgeCode>& dfsc)
    {
	std::copy(dfsc.begin(), dfsc.end(), std::ostream_iterator<EdgeCode>(out, " "));
	return out;
    }


    // *****************************************************************************
    //                          RMPath
    // *****************************************************************************
    class RMPath
    {
	std::deque<unsigned int> rmp_;
    public:
	template<class EdgeCode>
	explicit RMPath(const DFSCode_<EdgeCode>& dfsc);
	unsigned int operator[] (int i) const { return rmp_[i]; }
	unsigned int size() const { return rmp_.size(); }
	unsigned int rightmost() const { return rmp_.back(); }
	friend std::ostream& operator<<(std::ostream& out, const RMPath& rpm);
    };

    template<class EdgeCode>
    RMPath::RMPath(const DFSCode_<EdgeCode>& dfsc)
    {
	typename EdgeCode::VI old_from = 0;
	for (int i = dfsc.size()-1; i >= 0; --i)
	    if (dfsc[i].is_forward() && (rmp_.empty() || old_from == dfsc[i].vi_to))
	    {
		rmp_.push_front(i);
		old_from = dfsc[i].vi_from;
	    }
    }

    inline std::ostream& operator<<(std::ostream& out, const RMPath& r)
    {
	std::copy(r.rmp_.begin(), r.rmp_.end(), std::ostream_iterator<unsigned int>(out, " "));
	return out;
    }


    // *****************************************************************************
    //                          SubgraphOfTheGraph
    // *****************************************************************************
    template<class Edge, class Graph>
    class SubgraphOfTheGraph
    {
	std::vector<Edge> sbg_edges_;
	std::vector<int> vv_;
	std::vector<bool> ee_;
	const Graph* graph_;
    public:
	SubgraphOfTheGraph(const Graph* g) :graph_(g) {}
	void push(const Edge& edge);
	void pop();
	int size() const { return sbg_edges_.size(); }
	const Edge& operator[] (int i) const { return sbg_edges_[i]; }
	const Edge& get_edge(int i) const    { return sbg_edges_[i]; }
	bool has_vertex(typename Edge::VI vi) const { return vv_[vi]; }
	bool has_edge(const Edge& edge) const { return ee_[edge.ei]; }
	const Graph& get_graph() const { return *graph_; }
	const Graph* get_graph_p() const { return graph_; }

	template<class E, class G>
	friend std::ostream& operator<<(std::ostream& out, const SubgraphOfTheGraph<E,G>& r);
    };

    template<class Edge, class Graph>
    void SubgraphOfTheGraph<Edge,Graph>::push(const Edge& edge)
    {
	if (sbg_edges_.empty())
	{
	    sbg_edges_.reserve(num_edges(*graph_));
	    vv_.resize(num_vertices(*graph_), 0);
	    ee_.resize(num_edges(*graph_), false);
	}
	sbg_edges_.push_back(edge);
	++vv_[edge.vi_from];
	++vv_[edge.vi_to];
	ee_[edge.ei] = true;
    }

    template<class Edge, class Graph>
    void SubgraphOfTheGraph<Edge,Graph>::pop()
    {
	Edge edge = sbg_edges_.back();
	sbg_edges_.pop_back();
	--vv_[edge.vi_from];
	--vv_[edge.vi_to];
	ee_[edge.ei] = false;

	assert(vv_[edge.vi_from] >= 0);
	assert(vv_[edge.vi_to] >= 0);
    }

    template<class E, class G>
    std::ostream& operator<<(std::ostream& out, const SubgraphOfTheGraph<E,G>& r)
    {
	std::copy(r.sbg_edges_.begin(), r.sbg_edges_.end(), std::ostream_iterator<E>(out, " "));
	out << std::endl;
	return out;
    }

    // *****************************************************************************
    //                          Traits1
    // *****************************************************************************
    template<class GraphOps>
    struct Traits1
    {
	typedef typename GraphOps::graph_t            G;
	typedef typename GraphOps::vertex_index_t     VI;
	typedef typename GraphOps::edge_index_t       EI;
	typedef typename GraphOps::vertex_label_t     VL;
	typedef typename GraphOps::vertex_label_ref_t VLR;
	typedef typename GraphOps::edge_label_t       EL;
	typedef typename GraphOps::edge_label_ref_t   ELR;

	typedef Edge_<VI,EI> Edge;
	typedef EdgeCode_<VI,EI,VL,EL,VLR,ELR> EdgeCode;
	typedef DFSCode_<EdgeCode> DFSCode;
	typedef SubgraphOfTheGraph<Edge,G> SBG;
    };


    // *****************************************************************************
    //                          EdgeIterator
    // *****************************************************************************

    // -----------------------------------------------------
    //          EdgeIterator
    // -----------------------------------------------------
    template<class GraphOps, class DirTag>
    class EdgeIterator
    {
    public:
	typedef typename GraphOps::graph_t        Graph;
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;	
	typedef typename Traits1<GraphOps>::Edge Edge;
	EdgeIterator(typename Traits1<GraphOps>::VI vi, const Graph& graph, const GraphOps& ops);
	bool is_end() const { return ou_iters_.first == ou_iters_.second; }
	const Edge& get_edge() const { return curr_edge_; }
	void increment();
    private:
	typedef typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator;
	std::pair<out_edge_iterator,out_edge_iterator> ou_iters_;
	const Graph& graph_;
	void set_edge_ou()
	    {
		curr_edge_.vi_from = source(*ou_iters_.first, graph_);
		curr_edge_.vi_to   = target(*ou_iters_.first, graph_);
		curr_edge_.ei      = ops_.get_edge_index(*ou_iters_.first, graph_);
	    }
    protected:
	Edge curr_edge_;
	const GraphOps& ops_;
    };
    

    template<class GraphOps, class DirTag>
    EdgeIterator<GraphOps,DirTag>::EdgeIterator(typename Traits1<GraphOps>::VI vi,
						const Graph& graph,
						const GraphOps& ops)
	:ou_iters_(out_edges(ops.get_vertex_descriptor(vi,graph), graph)),
	 graph_(graph), ops_(ops)
    {
	if (ou_iters_.first != ou_iters_.second)
	    set_edge_ou();
	else
	{
	    curr_edge_.vi_from = ops_.void_vertex_index();
	    curr_edge_.vi_to   = ops_.void_vertex_index();
	    curr_edge_.ei      = ops_.void_edge_index();
	}
    }

    template<class GraphOps, class DirTag>
    void EdgeIterator<GraphOps,DirTag>::increment()
    {
	if (++ou_iters_.first != ou_iters_.second)
	    set_edge_ou();
    }


    // -----------------------------------------------------
    //		EdgeIterator for boost::bidirectionalS
    // -----------------------------------------------------
    template<class GraphOps>
    class EdgeIterator<GraphOps, boost::bidirectional_tag>
    {
    public:
	typedef typename GraphOps::graph_t        Graph;
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef typename boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
	typedef typename Traits1<GraphOps>::Edge Edge;

	EdgeIterator(vertex_descriptor vd, const Graph& graph, const GraphOps& ops);
	bool is_end() const { return state_ == END; }
	const Edge& get_edge() const { return curr_edge_; }
	void increment();
	const Graph& get_graph() const { }
    private:
	typedef typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator;
	typedef typename boost::graph_traits<Graph>::in_edge_iterator  in_edge_iterator;
	std::pair<out_edge_iterator,out_edge_iterator> ou_iters_;
	std::pair<in_edge_iterator,in_edge_iterator>   in_iters_;
	const Graph& graph_;
	enum State { OUT_ITER, IN_ITER, END } state_;
	void set_edge_ou()
	    {
		curr_edge_.vi_from = source(*ou_iters_.first, graph_);
		curr_edge_.vi_to   = target(*ou_iters_.first, graph_);
		curr_edge_.ei      = ops_.get_edge_index(*ou_iters_.first, graph_);
	    }
	void set_edge_in()
	    {
		curr_edge_.vi_from = target(*in_iters_.first, graph_);
		curr_edge_.vi_to   = source(*in_iters_.first, graph_);
		curr_edge_.ei      = ops_.get_edge_index(*in_iters_.first, graph_);
	    }
    protected:
	Edge curr_edge_;
	const GraphOps& ops_;
    };


    template<class GraphOps>
    EdgeIterator<GraphOps, boost::bidirectional_tag>::EdgeIterator(vertex_descriptor vd,
								   const Graph& graph,
								   const GraphOps& ops)
	:ou_iters_(out_edges(vd, graph)),
	 in_iters_(in_edges(vd, graph)),
	 graph_(graph), ops_(ops)
    {
	if (ou_iters_.first != ou_iters_.second)
	{
	    set_edge_ou();
	    state_ = OUT_ITER;
	}
	else if (in_iters_.first != in_iters_.second)
	{
	    set_edge_in();
	    state_ = IN_ITER;
	}
	else
	{
	    curr_edge_.vi_from = ops_.void_vertex_index();
	    curr_edge_.vi_to   = ops_.void_vertex_index();
	    curr_edge_.ei      = ops_.void_edge_index();
	    state_ = END;
	}
    }

    template<class GraphOps>
    void EdgeIterator<GraphOps, boost::bidirectional_tag>::increment()
    {
	switch (state_)
	{
	case OUT_ITER:
	    if (++ou_iters_.first != ou_iters_.second)
		set_edge_ou();
	    else
		if (in_iters_.first != in_iters_.second)
		{
		    set_edge_in();
		    state_ = IN_ITER;
		}
		else state_ = END;
	    break;

	case IN_ITER: 
	    if (++in_iters_.first != in_iters_.second)
		set_edge_in();
	    else
		state_ = END;
	    break;

	case END: assert(0); break;
	}
    }

    template<class GraphOps>
    struct EdgeIteratorSelector
    {
	typedef typename GraphOps::graph_t G;
	typedef typename boost::graph_traits<G>::directed_category DirTag;
	typedef EdgeIterator<GraphOps,DirTag> Type;
    };


    // -----------------------------------------------------
    //		EdgeIterator forward pure
    // -----------------------------------------------------
    template<class GraphOps>
    class ForwardPureEdgeIterator : public EdgeIteratorSelector<GraphOps>::Type
    {
	typedef typename EdgeIteratorSelector<GraphOps>::Type Base;
	using Base::curr_edge_;
	using Base::ops_;

	typename Traits1<GraphOps>::VLR minlabel_;
	const typename Traits1<GraphOps>::SBG* sbg_;
	void next_();
    public:
	ForwardPureEdgeIterator(const typename Traits1<GraphOps>::Edge& e,
				typename Traits1<GraphOps>::VLR minlabel,
				const typename Traits1<GraphOps>::SBG* sbg,
				const GraphOps& ops)
	    :Base(e.vi_to, sbg->get_graph(), ops),
	     minlabel_(minlabel),
	     sbg_(sbg)
	    { next_(); }
	void next() { this->increment(); next_(); }
    };

    template<class GraphOps>
    void ForwardPureEdgeIterator<GraphOps>::next_()
    {
	const typename Traits1<GraphOps>::G& g = sbg_->get_graph();
	for (; !this->is_end(); this->increment())
	{
	    if (ops_.vlabel_less_or_equal(minlabel_, ops_.vilabel(curr_edge_.vi_to, g)) &&
		!sbg_->has_vertex(curr_edge_.vi_to))
		break;
	}
    }

    // -----------------------------------------------------
    //		EdgeIterator forward rmpath
    // -----------------------------------------------------
    template<class GraphOps>
    class ForwardRMPathEdgeIterator : public EdgeIteratorSelector<GraphOps>::Type
    {
	typedef typename EdgeIteratorSelector<GraphOps>::Type Base;
	using Base::curr_edge_;
	using Base::ops_;

	typename Traits1<GraphOps>::VLR minlabel_;
	const typename Traits1<GraphOps>::SBG* sbg_;
	typename Traits1<GraphOps>::Edge edge_;
	void next_();
    public:
	ForwardRMPathEdgeIterator(const typename Traits1<GraphOps>::Edge& e,
				  typename Traits1<GraphOps>::VLR minlabel,
				  const typename Traits1<GraphOps>::SBG* sbg,
				  const GraphOps& ops)
	    :Base(e.vi_from, sbg->get_graph(), ops),
	     minlabel_(minlabel),
	     sbg_(sbg), edge_(e)
	    { next_(); }
	void next() { this->increment(); next_(); }
    };

    template<class GraphOps>
    void ForwardRMPathEdgeIterator<GraphOps>::next_()
    {
	const typename Traits1<GraphOps>::G& g = sbg_->get_graph();
	for (; !this->is_end(); this->increment())
	{
	    const typename Traits1<GraphOps>::Edge& e = curr_edge_;

	    typename Traits1<GraphOps>::VLR tolabel  = ops_.vilabel(edge_.vi_to, g);
	    typename Traits1<GraphOps>::VLR tolabel2 = ops_.vilabel(e.vi_to, g);

	    if (sbg_->has_edge(e) || sbg_->has_vertex(e.vi_to) || ops_.vlabel_less(tolabel2, minlabel_))
		continue;

	    typename Traits1<GraphOps>::ELR elabel  = ops_.eilabel(edge_.ei, g);
	    typename Traits1<GraphOps>::ELR elabel2 = ops_.eilabel(e.ei, g);

	    if (ops_.elabel_less(elabel, elabel2) ||
		(ops_.elabel_equal(elabel, elabel2) && ops_.vlabel_less_or_equal(tolabel, tolabel2) ) )
		break;
	}
    }


    // -----------------------------------------------------
    //		EdgeIterator backward
    // -----------------------------------------------------
    template<class GraphOps>
    class BackwardEdgeIterator : public EdgeIteratorSelector<GraphOps>::Type
    {
	typedef typename EdgeIteratorSelector<GraphOps>::Type Base;
	using Base::curr_edge_;
	using Base::ops_;
    public:
	BackwardEdgeIterator(const typename Traits1<GraphOps>::Edge& e1,
			     const typename Traits1<GraphOps>::Edge& e2,
			     const typename Traits1<GraphOps>::SBG* sbg,
			     const GraphOps& ops);
    };

    template<class GraphOps>
    BackwardEdgeIterator<GraphOps>::BackwardEdgeIterator(const typename Traits1<GraphOps>::Edge& e1,
							 const typename Traits1<GraphOps>::Edge& e2,
							 const typename Traits1<GraphOps>::SBG* sbg,
							 const GraphOps& ops)
	:Base(e2.vi_to, sbg->get_graph(), ops)
    {
	//assert(e1.ei != e2.ei); // ???????????????
	const typename Traits1<GraphOps>::G& g = sbg->get_graph();
	typename Traits1<GraphOps>::ELR e1label = ops_.eilabel(e1.ei, sbg->get_graph());
	bool b = ops.vlabel_less_or_equal(ops.vilabel(e1.vi_to, g),
					  ops.vilabel(e2.vi_to, g));

	for (; !this->is_end(); this->increment())
	{
	    const typename Traits1<GraphOps>::Edge& e = curr_edge_;
	    if (sbg->has_edge(e))
		continue;
	    typename Traits1<GraphOps>::ELR elabel = ops.eilabel(e.ei,g);
	    if (e.vi_to == e1.vi_from &&
		(ops.elabel_less(e1label,elabel) || (ops.elabel_equal(e1label,elabel) && b) ) )
		break;
	}
    }

    // *****************************************************************************
    //                          Traits2
    // *****************************************************************************
    template<class GraphOps>
    struct Traits2 : public Traits1<GraphOps>
    {
	typedef std::list<typename Traits1<GraphOps>::SBG> SBGS;
	typedef std::pair<typename Traits1<GraphOps>::SBG*, typename Traits1<GraphOps>::Edge> ExtEdge;
	typedef std::list<ExtEdge> ExtEdgeList;
    };


    // *****************************************************************************
    //                          Projected
    // *****************************************************************************
    template<class GraphOps>
    class Projected_ : public std::vector<typename Traits2<GraphOps>::SBG* >
    {
    public:
	typedef typename Traits2<GraphOps>::ExtEdgeList ExtEdgeList;
	Projected_() {}
	explicit Projected_(const typename Traits2<GraphOps>::ExtEdgeList& exl);
	~Projected_();
    };


    template<class GraphOps>
    Projected_<GraphOps>::Projected_(const typename Traits2<GraphOps>::ExtEdgeList& exl)
    {
	reserve(exl.size());
	for (typename ExtEdgeList::const_iterator i = exl.begin(); i != exl.end(); ++i)
	{
	    i->first->push(i->second);
	    push_back(i->first);
	}
    }

    template<class GraphOps>
    Projected_<GraphOps>::~Projected_()
    {
	typedef std::vector<typename Traits2<GraphOps>::SBG* > Base;
	for (typename Base::iterator i = this->begin(); i != this->end(); ++i)
	    (*i)->pop();
    }

    template<class GraphOps>
    int num_graphs(const Projected_<GraphOps>& j)
    {
	std::set<const typename Traits2<GraphOps>::G*> n;
	BOOST_FOREACH(const typename Traits2<GraphOps>::SBG* p, j)
	    n.insert(p->get_graph_p());
	return n.size();
    }


    template<class GraphOps>
    std::ostream& operator<<(std::ostream& out, const Projected_<GraphOps>& r)
    {
	BOOST_FOREACH(const typename Traits2<GraphOps>::SBG* p, r) out << *p << std::endl;
	return out;
    }


    // *****************************************************************************
    //                          Maps
    // *****************************************************************************

    template<class M1>
    inline typename M1::mapped_type&
    getval(M1& mp1key, const typename M1::key_type& k1)
    {
	typedef typename M1::value_type value_type;
	return mp1key.insert(value_type(k1, typename M1::mapped_type())).first->second;
    }

    template<class M2>
    inline typename M2::mapped_type::mapped_type&
    getval(M2& mp2key,
	   const typename M2::key_type& k1,
	   const typename M2::mapped_type::key_type& k2,
	   const typename M2::mapped_type::key_compare& c2)
    {
	typedef typename M2::value_type value_type;
	return getval(mp2key.insert(value_type(k1, typename M2::mapped_type(c2))).first->second, k2);
    }


    template<class M3>
    inline typename M3::mapped_type::mapped_type::mapped_type&
    getval(M3& mp3key,
	   const typename M3::key_type& k1,
	   const typename M3::mapped_type::key_type& k2,
	   const typename M3::mapped_type::mapped_type::key_type& k3,
	   const typename M3::mapped_type::key_compare& c2,
	   const typename M3::mapped_type::mapped_type::key_compare& c3)
    {
	typedef typename M3::value_type value_type;
	return getval(mp3key.insert(value_type(k1, typename M3::mapped_type(c2))).first->second, k2, k3, c3);
    }

    
    template<class GraphOps>
    class Vlabel_less_
    {
	const GraphOps& ops_;
    public:
	explicit Vlabel_less_(const GraphOps& ops) :ops_(ops) {}
	bool operator() (typename Traits1<GraphOps>::VLR vl1, typename Traits1<GraphOps>::VLR vl2) const
	    { return ops_.vlabel_less(vl1, vl2); }
    };

    template<class GraphOps>    
    class Elabel_less_
    {
	const GraphOps& ops_;
    public:
	explicit Elabel_less_(const GraphOps& ops) :ops_(ops) {}
	bool operator() (typename Traits1<GraphOps>::ELR el1, typename Traits1<GraphOps>::ELR el2) const
	    { return ops_.elabel_less(el1, el2); }
    };


    // *****************************************************************************
    //                          Traits3
    // *****************************************************************************
    template<class GraphOps>
    class Traits3 : public Traits2<GraphOps>
    {
	// std::map wrapper without default constructor
	template<class K, class C, class T>
	struct map_ : public std::map<K,T,C> { explicit map_(const C& c) :std::map<K,T,C>(c) {} };
    public:

	typedef typename Traits1<GraphOps>::VI VI;
	typedef typename Traits1<GraphOps>::VL VL;
	typedef typename Traits1<GraphOps>::EL EL;

	typedef std::less<VI> VI_less;
	typedef Vlabel_less_<GraphOps> Vlabel_less;
	typedef Elabel_less_<GraphOps> Elabel_less;

	// ExtEdgeList maps
	typedef typename Traits2<GraphOps>::ExtEdgeList         ExtEdgeList;
	typedef map_<EL, Elabel_less, ExtEdgeList>		Map_EL_ExtEdgeList;
	typedef map_<VL, Vlabel_less, ExtEdgeList>		Map_VL_ExtEdgeList;
	typedef map_<EL, Elabel_less, Map_VL_ExtEdgeList>	Map_EL_VL_ExtEdgeList;
	typedef map_<VI, VI_less,     Map_EL_ExtEdgeList>	Map_VI_EL_ExtEdgeList;
	typedef map_<VI, VI_less,     Map_EL_VL_ExtEdgeList>	Map_VI_EL_VL_ExtEdgeList;

	// Projected map
	typedef Projected_<GraphOps> Projected;
	typedef map_<VL, Vlabel_less,
		     map_<EL, Elabel_less,
			  map_<VL, Vlabel_less, Projected> > >  Map_VL_EL_VL_P;
    };


    // *****************************************************************************
    //                          gspan functions
    // *****************************************************************************
    template<class Projected>
    inline int support(const Projected& projected)
    {
	return num_graphs(projected);
    }

    template<class Projected, class DFSCode, class Output>
    void report(const Projected& projected, const DFSCode& dfsc, Output& result)
    {
	result(projected, dfsc);
    }


    template<class GraphOps>
    void enumerate_one(typename Traits3<GraphOps>::Map_VL_EL_VL_P& m,
		       typename Traits3<GraphOps>::SBGS& sbgs,
		       const typename Traits3<GraphOps>::G& g,
		       const GraphOps& ops)
    {
	typename Traits3<GraphOps>::Vlabel_less vl_less(ops);
	typename Traits3<GraphOps>::Elabel_less el_less(ops);

	typedef typename Traits3<GraphOps>::SBG SBG;
	typedef typename Traits3<GraphOps>::G   G;
	typename boost::graph_traits<G>::vertex_iterator vi, viend;
	for (boost::tie(vi,viend) = vertices(g); vi != viend; ++vi)
	{
	    typedef typename EdgeIteratorSelector<GraphOps>::Type EdgeIter;
	    for (EdgeIter iter(*vi, g, ops); !iter.is_end(); iter.increment())
	    {
		typename Traits3<GraphOps>::Edge e = iter.get_edge();
		sbgs.push_back(SBG(&g));
		SBG* sbg = &sbgs.back();
		sbg->push(e);
		typename Traits3<GraphOps>::VLR vl_from = ops.vilabel(e.vi_from, g);
		typename Traits3<GraphOps>::VLR vl_to   = ops.vilabel(e.vi_to, g);
	        typename Traits3<GraphOps>::ELR el      = ops.eilabel(e.ei, g);

		assert(vl_from != "");
		assert(! ops.void_vlabel(vl_from));
		assert(! ops.void_vlabel(vl_to));

		getval(m,
		       vl_from, el, vl_to,
		       el_less, vl_less).push_back(sbg);
	    }
	}
    }

    template<class GraphOps>
    bool project_is_min(const typename Traits3<GraphOps>::Projected& projected,
			typename Traits3<GraphOps>::DFSCode& dfsc_min,
			const typename Traits3<GraphOps>::DFSCode& dfsc_tested,
			const GraphOps& ops)
    {
	typedef typename Traits3<GraphOps>::VLR VLR;
	typedef typename Traits3<GraphOps>::ELR ELR;
	typedef typename Traits3<GraphOps>::SBG SBG;
	typedef typename Traits3<GraphOps>::Edge Edge;
	typedef typename Traits3<GraphOps>::ExtEdge ExtEdge;
	typedef typename Traits3<GraphOps>::EdgeCode EdgeCode;
	
	if (! edgecode_equal(dfsc_min[dfsc_min.size()-1], dfsc_tested[dfsc_min.size()-1], ops))
	    return false;

	// --------------------------------------------------------------
	// enumerate
	typedef typename Traits3<GraphOps>::Map_EL_ExtEdgeList    BckEdges;
	typedef typename Traits3<GraphOps>::Map_EL_VL_ExtEdgeList FwdEdges;

	RMPath rmpath(dfsc_min);
	typename Traits1<GraphOps>::VI maxtoc = dfsc_min[rmpath.rightmost()].vi_to;

	typename Traits3<GraphOps>::Elabel_less el_less(ops);
	typename Traits3<GraphOps>::Vlabel_less vl_less(ops);

	// backward
	{
	    typename Traits3<GraphOps>::Elabel_less el_less(ops);
	    BckEdges bck_edges(el_less);
	    typename Traits1<GraphOps>::VI newto = ops.void_vertex_index();
	    bool flg = false;
	    for (unsigned int i = 0; !flg && i < rmpath.size() - 1; ++i)
	    {
		BOOST_FOREACH(SBG* sbg, projected)
		{
		    SBG& s = *sbg;
		    const typename Traits3<GraphOps>::G& g = s.get_graph();
		    BackwardEdgeIterator<GraphOps> iter(s[rmpath[i]], s[rmpath.rightmost()], sbg, ops);
		    if (!iter.is_end())
		    {
			const Edge& e = iter.get_edge();
			ExtEdge ext(sbg, e);
			ELR el = ops.eilabel(e.ei,g);
			getval(bck_edges, el).push_back(ext);
			newto = dfsc_min[rmpath[i]].vi_from;
			flg = true;
		    }
		}
	    }

	    if (flg)
	    {
		typename BckEdges::const_iterator i1 = bck_edges.begin();
		dfsc_min.push(EdgeCode(maxtoc, newto, ops.void_vlabel(), i1->first, ops.void_vlabel()));
		typename Traits3<GraphOps>::Projected next_projected(i1->second);
		return project_is_min(next_projected, dfsc_min, dfsc_tested, ops);
	    }
	}

	// forward
	{
	    FwdEdges fwd_edges(el_less);
	    typename Traits3<GraphOps>::VL minlabel = dfsc_min[0].vl_from;
	    typename Traits3<GraphOps>::VI newfrom = ops.void_vertex_index();
	    bool flg = false;
	    
	    // forward pure
	    BOOST_FOREACH(SBG* sbg, projected)
	    {
		SBG& s = *sbg;
		const typename Traits3<GraphOps>::G& g = s.get_graph();
		for (ForwardPureEdgeIterator<GraphOps> iter(s[rmpath.rightmost()], minlabel, sbg, ops);
		     !iter.is_end(); iter.next())
		{
		    const Edge& e = iter.get_edge();
		    ExtEdge ext(sbg, e);
		    ELR el = ops.eilabel(e.ei,g);
		    VLR vl = ops.vilabel(e.vi_to,g);
		    getval(fwd_edges, el, vl, vl_less).push_back(ext);
		    newfrom = maxtoc;
		    flg = true;
		}
	    }

	    // forward rmpath
	    for (int i = rmpath.size()-1; !flg && i >= 0; --i)
	    {
		BOOST_FOREACH(SBG* sbg, projected)
		{
		    SBG& s = *sbg;
		    const typename Traits3<GraphOps>::G& g = s.get_graph();
		    for (ForwardRMPathEdgeIterator<GraphOps> iter(s[rmpath[i]], minlabel, sbg, ops);
			 !iter.is_end(); iter.next())
		    {
			const Edge& e = iter.get_edge();
			ExtEdge ext(sbg, e);
			ELR el = ops.eilabel(e.ei,g);
			VLR vl = ops.vilabel(e.vi_to,g);
			getval(fwd_edges, el, vl, vl_less).push_back(ext);
			newfrom = dfsc_min[rmpath[i]].vi_from;
			flg = true;
		    }
		}
	    }

	    if (flg)
	    {
		typename FwdEdges::const_iterator i1 = fwd_edges.begin();
		typename FwdEdges::mapped_type::const_iterator i2 = i1->second.begin();
		dfsc_min.push(EdgeCode(newfrom, maxtoc+1, ops.void_vlabel(), i1->first, i2->first));
		typename Traits3<GraphOps>::Projected next_projected(i2->second);
		return project_is_min(next_projected, dfsc_min, dfsc_tested, ops);
	    }
	}

	return true;
    }

    template<class GraphOps>
    bool is_min(const typename Traits3<GraphOps>::DFSCode& dfsc_tested, const GraphOps& ops)
    {
	typedef typename Traits3<GraphOps>::G   G;
	typedef typename Traits3<GraphOps>::SBGS SBGS;
	typedef typename Traits3<GraphOps>::Map_VL_EL_VL_P M3;
	
	typename Traits3<GraphOps>::Vlabel_less vl_less(ops);

	std::auto_ptr<G> graph(ops.create_graph(dfsc_tested));	
	SBGS sbgs;
	M3 root(vl_less);
	enumerate_one(root, sbgs, *graph, ops);
	
	typename Traits3<GraphOps>::DFSCode dfsc_min;
	typename M3::const_iterator i1 = root.begin();
	typename M3::mapped_type::const_iterator i2 = i1->second.begin();
	typename M3::mapped_type::mapped_type::const_iterator i3 = i2->second.begin();
	typename Traits3<GraphOps>::EdgeCode ec(0, 1, i1->first, i2->first, i3->first);
	dfsc_min.push(ec);
	return project_is_min(i3->second, dfsc_min, dfsc_tested, ops);
    }


    template<class GraphOps, class Output>
    void project(const typename Traits3<GraphOps>::Projected& projected,
		 typename Traits3<GraphOps>::DFSCode& dfsc, int minsup,
		 const GraphOps& ops, Output& result)
    {
	typedef typename Traits3<GraphOps>::VI  VI;
	typedef typename Traits3<GraphOps>::VLR VLR;
	typedef typename Traits3<GraphOps>::ELR ELR;
	typedef typename Traits3<GraphOps>::SBG SBG;
	typedef typename Traits3<GraphOps>::Edge Edge;
	typedef typename Traits3<GraphOps>::ExtEdge ExtEdge;
	typedef typename Traits3<GraphOps>::EdgeCode EdgeCode;

	int sup = support(projected);
	if (sup < minsup)
	    return;

	if (! is_min(dfsc, ops))
	    return;
	
	report(projected, dfsc, result);
	
	// --------------------------------------------------------------
	// enumerate
	typedef typename Traits3<GraphOps>::Map_VI_EL_ExtEdgeList    BckEdges;
	typedef typename Traits3<GraphOps>::Map_VI_EL_VL_ExtEdgeList FwdEdges;

	std::less<VI> vi_less;
	typename Traits3<GraphOps>::Elabel_less el_less(ops);
	typename Traits3<GraphOps>::Vlabel_less vl_less(ops);

	BckEdges bck_edges(vi_less);
	FwdEdges fwd_edges(vi_less);

	RMPath rmpath(dfsc);

	VI maxtoc   = dfsc[rmpath.rightmost()].vi_to;
	VLR minlabel = dfsc[0].vl_from;
	
	BOOST_FOREACH(SBG* sbg, projected)
	{
	    SBG& s = *sbg;
	    const typename Traits3<GraphOps>::G& g = s.get_graph();

	    // backward
	    for (unsigned int i = 0; i < rmpath.size() - 1; ++i)
	    {
		BackwardEdgeIterator<GraphOps> iter(s[rmpath[i]], s[rmpath.rightmost()], sbg, ops);
		if (!iter.is_end())
		{
		    const Edge& e = iter.get_edge();
		    ExtEdge ext(sbg, e);
		    VI vi  = dfsc[rmpath[i]].vi_from;
		    ELR el = ops.eilabel(e.ei,g);
		    getval(bck_edges, vi, el, el_less).push_back(ext);
		}
	    }

	    // forward
	    for (ForwardPureEdgeIterator<GraphOps> iter(s[rmpath.rightmost()], minlabel, sbg, ops);
		 !iter.is_end(); iter.next())
	    {
		const Edge& e = iter.get_edge();
		ExtEdge ext(sbg, e);
		ELR el = ops.eilabel(e.ei,g);
		VLR vl = ops.vilabel(e.vi_to,g);
		getval(fwd_edges, maxtoc, el, vl, el_less, vl_less).push_back(ext);
	    }

	    for (int i = rmpath.size()-1; i >= 0; --i)
		for (ForwardRMPathEdgeIterator<GraphOps> iter(s[rmpath[i]], minlabel, sbg, ops);
		     !iter.is_end(); iter.next())
		{
		    const Edge& e = iter.get_edge();
		    ExtEdge ext(sbg, e);
		    VI vi  = dfsc[rmpath[i]].vi_from;
		    ELR el = ops.eilabel(e.ei,g);
		    VLR vl = ops.vilabel(e.vi_to,g);
		    getval(fwd_edges, vi, el, vl, el_less, vl_less).push_back(ext);
		}
	}

	// --------------------------------------------------------------
	// test SBG + extended edge children

	// backward
	for (typename BckEdges::const_iterator it1 = bck_edges.begin(); it1 != bck_edges.end(); ++it1)
	    for (typename BckEdges::mapped_type::const_iterator it2 = it1->second.begin();
		 it2 != it1->second.end(); ++it2)
	    {
		EdgeCode ec(maxtoc, it1->first, ops.void_vlabel(), it2->first, ops.void_vlabel());
		dfsc.push(ec);
		typename Traits3<GraphOps>::Projected next_projected(it2->second);
		project(next_projected, dfsc, minsup, ops, result);
		dfsc.pop();
	    }

	// forward
	for (typename FwdEdges::const_reverse_iterator it1 = fwd_edges.rbegin();
	     it1 != fwd_edges.rend(); ++it1)
	    for (typename FwdEdges::mapped_type::const_iterator it2 = it1->second.begin();
		 it2 != it1->second.end(); ++it2)
		for (typename FwdEdges::mapped_type::mapped_type::const_iterator it3 = it2->second.begin();
		     it3 != it2->second.end(); ++it3)
		{
		    EdgeCode ec(it1->first, maxtoc+1, ops.void_vlabel(), it2->first, it3->first);
		    dfsc.push(ec);
		    typename Traits3<GraphOps>::Projected next_projected(it3->second);
		    project(next_projected, dfsc, minsup, ops, result);
		    dfsc.pop();
		}
    }


    template<class TGraphIterator, class GraphOps, class Output>
    void gspan(TGraphIterator tg_begin, TGraphIterator tg_end, int minsup,
	       const GraphOps& ops, Output& result)
    {
	typedef typename Traits3<GraphOps>::G   G;
	typedef typename Traits3<GraphOps>::SBGS SBGS;
	typedef typename Traits3<GraphOps>::Map_VL_EL_VL_P M3;

	typename Traits3<GraphOps>::Vlabel_less vl_less(ops);
	SBGS sbgs;
	M3 root(vl_less);
	for (; tg_begin != tg_end; ++tg_begin)
	    enumerate_one(root, sbgs, *tg_begin, ops);

	typename Traits3<GraphOps>::DFSCode dfsc;

	for (typename M3::const_iterator i1 = root.begin(); i1 != root.end(); ++i1)
	    for (typename M3::mapped_type::const_iterator i2 = i1->second.begin();
		 i2 != i1->second.end(); ++i2)
		for (typename M3::mapped_type::mapped_type::const_iterator i3 = i2->second.begin();
		     i3 != i2->second.end(); ++i3)
		{
		    typename Traits3<GraphOps>::EdgeCode ec(0, 1, i1->first, i2->first, i3->first);
#ifdef DEBUG_PRINT
		    std::cerr << "gspan(): top level iteration with edgecode: " << ec << std::endl;
#endif
		    dfsc.push(ec);
		    project(i3->second, dfsc, minsup, ops, result);
		    dfsc.pop();
		}
    }

}
#endif
