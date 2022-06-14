/*=============================================================================
# Filename: QueryTree.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description: 
=============================================================================*/

#ifndef _QUERY_QUERYTREE_H
#define _QUERY_QUERYTREE_H

#include "../Util/Util.h"
#include "Varset.h"

class QueryTreeRelabeler
{
    public:
    std::string suffix;
    Varset excluding;
    void relabel(std::string& str){if(!excluding.findVar(str)) str+=suffix;}
    void relabel_safe(std::string& str){if(str.length()!=0&&str[0]=='?'&&!excluding.findVar(str)) str+=suffix;}
    void relabel(std::vector<std::string>& strs){for(std::vector<std::string>::iterator it=strs.begin(); it!=strs.end(); it++) relabel(*it);}
    void relabel(Varset& var){relabel(var.vars);}
    QueryTreeRelabeler(std::string suf):suffix(suf){}
};


class QueryTree;
class GroupPattern;
class PathArgs;
class CompTreeNode;
class ProjectionVar;
class Order;
class GroupPattern
		{
			public:
				class Pattern;
				class FilterTree;
				class Bind;
				class SubGroupPattern;

				std::vector<SubGroupPattern> sub_group_pattern;
                bool pattern_only;

				Varset group_pattern_resultset_minimal_varset, group_pattern_resultset_maximal_varset;
				Varset group_pattern_subject_object_maximal_varset, group_pattern_predicate_maximal_varset;

				void addOnePattern(Pattern _pattern);

				void addOneGroup();
				GroupPattern& getLastGroup();

				void addOneGroupUnion();
				void addOneUnion();
				GroupPattern& getLastUnion();

				void addOneOptional(int _type);
				GroupPattern& getLastOptional();

				void addOneFilter();
				CompTreeNode& getLastFilter();

				void addOneBind();
				Bind& getLastBind();

				void addOneSubquery();
				QueryTree& getLastSubquery();

				void getVarset();

				std::pair<Varset, Varset> checkNoMinusAndOptionalVarAndSafeFilter(Varset occur_varset, Varset ban_varset, bool &check_condition);

				void initPatternBlockid();
				int getRootPatternBlockID(int x);
				void mergePatternBlockID(int x, int y);

				void print(int dep);
                //@@ GroupPattern(const GroupPattern& _gp):sub_group_pattern(_gp.sub_group_pattern), group_pattern_resultset_minimal_varset(_gp.group_pattern_resultset_minimal_varset), group_pattern_resultset_maximal_varset(_gp.group_pattern_resultset_maximal_varset), group_pattern_subject_object_maximal_varset(_gp.group_pattern_subject_object_maximal_varset), group_pattern_predicate_maximal_varset(_gp.group_pattern_predicate_maximal_varset), pattern_only(_gp.pattern_only){}
                GroupPattern():pattern_only(0){}
                void relabel(QueryTreeRelabeler& qtr);
                bool populate_pattern_only();
		};

class QueryTree
{
	public:
		enum QueryForm {Select_Query, Ask_Query};
		enum ProjectionModifier {Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates};
		enum UpdateType {Not_Update, Insert_Data, Delete_Data, Delete_Where, Insert_Clause, Delete_Clause, Modify_Clause};
		typedef ::GroupPattern GroupPattern;
        typedef ::ProjectionVar ProjectionVar;
        typedef ::CompTreeNode CompTreeNode;
        typedef ::Order Order;
		private:
			QueryForm query_form;

			ProjectionModifier projection_modifier;
			std::vector<ProjectionVar> projection;
			bool projection_asterisk;

			Varset group_by;
			std::vector<Order> order_by;
			int offset, limit;

			GroupPattern group_pattern;

			//----------------------------------------------------------------------------------------------------------------------------------------------------

			UpdateType update_type;

			//only use patterns
			GroupPattern insert_patterns, delete_patterns;

		public:
			QueryTree():
				query_form(Select_Query), projection_modifier(Modifier_None), projection_asterisk(false), offset(0), limit(-1), update_type(Not_Update){}
            QueryTree(const QueryTree& _qt): query_form(_qt.query_form), projection_modifier(_qt.projection_modifier), projection(_qt.projection), projection_asterisk(_qt.projection_asterisk), group_by(_qt.group_by), order_by(_qt.order_by), offset(_qt.offset), limit(_qt.limit), group_pattern(_qt.group_pattern), update_type(_qt.update_type), insert_patterns(_qt.insert_patterns), delete_patterns(_qt.delete_patterns){}

			void setQueryForm(QueryForm _queryform);
			QueryForm getQueryForm();

			void setProjectionModifier(ProjectionModifier _projection_modifier);
			ProjectionModifier getProjectionModifier();

			void addProjectionVar();
			ProjectionVar& getLastProjectionVar();
			std::vector<ProjectionVar>& getProjection();
			Varset getProjectionVarset();
			Varset getResultProjectionVarset();

			void setProjectionAsterisk();
			bool checkProjectionAsterisk();

			void addGroupByVar(std::string &_var);
			Varset& getGroupByVarset();

			// void addOrderVar(std::string &_var, bool _descending);
            void addOrderVar(bool _descending);
			std::vector<Order>& getOrderVarVector();
            Order& getLastOrderVar();
			Varset getOrderByVarset();

			void setOffset(int _offset);
			int getOffset();
			void setLimit(int _limit);
			int getLimit();

			GroupPattern& getGroupPattern();

			void setUpdateType(UpdateType _updatetype);
			UpdateType getUpdateType();
			GroupPattern& getInsertPatterns();
			GroupPattern& getDeletePatterns();

			bool checkWellDesigned();
			bool checkAtLeastOneAggregateFunction();
			bool checkSelectAggregateFunctionGroupByValid();

			void print();
            void relabel(QueryTreeRelabeler& qtr);
            void relabel_full(QueryTreeRelabeler& qtr);
};
		class GroupPattern::Pattern
		{
			public:
				class Element
				{
					public:
						//enum Type { Variable, Literal, IRI };
						//enum SubType { None, CustomLanguage, CustomType };
						//Type type;
						//SubType subType;
						//std::string subTypeValue;

						std::string value;
						Element(){}
						Element(const std::string &_value):value(_value){}
                        Element(const Element& _ele): value(_ele.value){}
				};
				Element subject, predicate, object;
				Varset varset, subject_object_varset;
				int blockid;

				Pattern():blockid(-1){}
				Pattern(const Element _subject, const Element _predicate, const Element _object):
					subject(_subject), predicate(_predicate), object(_object), blockid(-1){}
                Pattern(const Pattern& _pat): subject(_pat.subject), predicate(_pat.predicate), object(_pat.object), varset(_pat.varset), subject_object_varset(_pat.subject_object_varset), blockid(_pat.blockid){}

				bool operator < (const Pattern &x) const
				{
					if (this->subject.value != x.subject.value)
						return this->subject.value < x.subject.value;
					if (this->predicate.value != x.predicate.value)
						return this->predicate.value < x.predicate.value;
					return (this->object.value < x.object.value);
				}
                void relabel(QueryTreeRelabeler& qtr){
                    qtr.relabel(varset);
                    qtr.relabel(subject_object_varset);
                    qtr.relabel_safe(subject.value);
                    qtr.relabel_safe(predicate.value);
                    qtr.relabel_safe(object.value);
                }
		};

		class PathArgs
		{
		public:
			std::string src, dst;
			bool directed;
			std::vector<std::string> pred_set;
			int k;
			float confidence;
            int retNum;
		};

		class CompTreeNode
		{
		public:
			std::string oprt;	// operator
			//CompTreeNode *lchild;
			//CompTreeNode *rchild;	// child nodes
			std::vector<CompTreeNode> children;     // child nodes
			std::string val;	// variable, or literal followed by datatype suffix
			PathArgs path_args;
			Varset varset;
			bool done;

			CompTreeNode(): done(false) {}
			// CompTreeNode(const CompTreeNode& that);
			// CompTreeNode& operator=(const CompTreeNode& that);
			// ~CompTreeNode();
			void print(int dep);    // Print subtree rooted at this node
			Varset getVarset();
            void relabel(QueryTreeRelabeler& qtr){qtr.relabel(varset); qtr.relabel_safe(val); for(std::vector<CompTreeNode>::iterator it=children.begin(); it!=children.end(); it++) it->relabel(qtr);};
		};

		class GroupPattern::FilterTree
		{
			public:
				class FilterTreeNode
				{
					public:
						enum FilterOperationType
						{
							None_type, Or_type, And_type, Equal_type, NotEqual_type, Less_type, LessOrEqual_type, Greater_type, GreaterOrEqual_type,
							Plus_type, Minus_type, Mul_type, Div_type, Not_type, UnaryPlus_type, UnaryMinus_type, Literal_type, Variable_type, IRI_type,
							Function_type, ArgumentList_type, Builtin_str_type, Builtin_lang_type, Builtin_langmatches_type, Builtin_datatype_type, Builtin_bound_type,
							Builtin_sameterm_type, Builtin_isiri_type, Builtin_isuri_type, Builtin_isblank_type, Builtin_isliteral_type, Builtin_isnumeric_type,
							Builtin_regex_type, Builtin_in_type, Builtin_exists_type,
							Builtin_contains_type, Builtin_ucase_type, Builtin_lcase_type, Builtin_strstarts_type,
							Builtin_now_type, Builtin_year_type, Builtin_month_type, Builtin_day_type,
							Builtin_abs_type,
							Builtin_simpleCycle_type, Builtin_cycle_type, Builtin_sp_type, Builtin_khop_type
						};
						FilterOperationType oper_type;

						class FilterTreeChild;

						std::vector<FilterTreeChild> child;

						FilterTreeNode():oper_type(None_type){}
                        FilterTreeNode(const FilterTreeNode& _ftn):oper_type(_ftn.oper_type), child(_ftn.child){}

						void getVarset(Varset &varset);
						void mapVarPos2Varset(Varset &varset, Varset &entity_literal_varset);

						void print(int dep);
                        void relabel(QueryTreeRelabeler& qtr);
				};

				FilterTreeNode root;
				Varset varset;
				bool done;
				FilterTree():done(false){}
                //@@ FilterTree(const FilterTree& _ft):root(_ft.root), varset(_ft.varset), done(_ft.done){}
                void relabel(QueryTreeRelabeler& qtr){qtr.relabel(varset); root.relabel(qtr);};
		};

			class GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild
			{
				public:
					enum FilterTreeChildNodeType {None_type, Tree_type, String_type};
					FilterTreeChildNodeType node_type;

					FilterTreeNode node;
					std::string str;
					int pos;
					bool isel;

					PathArgs path_args;

					FilterTreeChild():node_type(None_type), pos(-1), isel(true){}
                    FilterTreeChild(const FilterTreeChild& _ftc):node_type(_ftc.node_type), node(_ftc.node), str(_ftc.str), pos(_ftc.pos), isel(_ftc.isel), path_args(_ftc.path_args){}
                    void relabel(QueryTreeRelabeler& qtr){
                        if (node_type == FilterTreeChild::String_type)
                        	qtr.relabel_safe(str);
                        if (node_type == FilterTreeChild::Tree_type)
                        	node.relabel(qtr);
                    };
			};

		class GroupPattern::Bind
		{
			public:
				Bind(){}
				Bind(const std::string &_str, const std::string &_var):str(_str), var(_var){}
				std::string str, var;
				Varset varset;
                //@@ Bind(const Bind& _b): str(_b.str), var(_b.var), varset(_b.varset){}
                void relabel(QueryTreeRelabeler& qtr){qtr.relabel(var); qtr.relabel(varset);}
		};

		class GroupPattern::SubGroupPattern
		{
			public:
				enum SubGroupPatternType{Group_type, Pattern_type, Union_type, Optional_type, Minus_type, Filter_type, Bind_type, Subquery_type};
				SubGroupPatternType type;

				Pattern pattern;	// triplesBlock

				// graphPatternNotTriples
				GroupPattern group_pattern;
				std::vector<GroupPattern> unions;	// groupOrUnionGraphPattern
				GroupPattern optional;	// optionalGraphPattern, minusGraphPattern
				// FilterTree filter;	// filter
				CompTreeNode filter;
				Bind bind;	// bind
                QueryTree subquery;

				SubGroupPattern(SubGroupPatternType _type):type(_type){}
				//@@ SubGroupPattern(const SubGroupPattern& _sgp):type(_sgp.type),pattern(_sgp.pattern),group_pattern(_sgp.group_pattern),unions(_sgp.unions),optional(_sgp.optional),filter(_sgp.filter),bind(_sgp.bind),subquery(_sgp.subquery){}
                void relabel(QueryTreeRelabeler& qtr);
                bool pattern_only(){return type==Pattern_type||(type==Group_type&&group_pattern.pattern_only);}
                bool subquery_only(){return type==Subquery_type||(type==Group_type&&group_pattern.sub_group_pattern.size()==1&&group_pattern.sub_group_pattern[0].subquery_only());}
		};

		class ProjectionVar
		{
			public:
				enum AggregateType {None_type, Count_type, Sum_type, Min_type, Max_type, Avg_type, 
					simpleCyclePath_type, simpleCycleBoolean_type, cyclePath_type, cycleBoolean_type, 
					shortestPath_type, shortestPathLen_type, kHopReachable_type, kHopEnumerate_type, 
					kHopReachablePath_type, ppr_type,
					CompTree_type, Contains_type, Custom_type};
				AggregateType aggregate_type;

				std::string var, aggregate_var;
				bool distinct;

				PathArgs path_args;

				CompTreeNode comp_tree_root;

				std::vector<std::string> func_args;
				std::string custom_func_name;

				ProjectionVar():aggregate_type(None_type), distinct(false){}
                //@@ ProjectionVar(const ProjectionVar& _pv):aggregate_type(_pv.aggregate_type), var(_pv.var), aggregate_var(_pv.aggregate_var), distinct(_pv.distinct), path_args(_pv.path_args), comp_tree_root(_pv.comp_tree_root), builtin_args(_pv.builtin_args){}
                void relabel(QueryTreeRelabeler& qtr){if(aggregate_type != ProjectionVar::None_type) qtr.relabel(var);}
                void relabel_full(QueryTreeRelabeler& qtr){qtr.relabel(aggregate_var); if(aggregate_type != ProjectionVar::None_type) qtr.relabel(var);}
		};

		class Order
		{
			public:
				std::string var;	// Don't remove for backward compatibility
				CompTreeNode comp_tree_root;	// Support expression
				bool descending;
				// Order(std::string &_var, bool _descending): var(_var), descending(_descending) { comp_tree_root = new CompTreeNode(); }
				Order(bool _descending);
				// Order(const Order& that);
				// Order& operator=(const Order& that);
				// ~Order();
                //@@ Order(const Order& _o):var(_o.var), descending(_o.descending), comp_tree_root(_o.comp_tree_root){}
                void relabel(QueryTreeRelabeler& qtr){qtr.relabel(var);comp_tree_root.relabel(qtr);}
		};
#endif // _QUERY_QUERYTREE_H

