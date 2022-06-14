/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

#include "../VSTree/VSTree.h"
#include "../KVstore/KVstore.h"
#include "../StringIndex/StringIndex.h"
#include "../Database/Strategy.h"
#include "../Database/CSR.h"
#include "../Parser/QueryParser.h"
#include "../Util/Triple.h"
#include "../Util/Util.h"
#include "../Util/EvalMultitypeValue.h"
#include "SPARQLquery.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "TempResult.h"
#include "QueryCache.h"
#include "ResultSet.h"
#include "PathQueryHandler.h"

class GeneralEvaluation
{
	private:
		shared_ptr<QueryParser> query_parser;
		QueryTree query_tree;
		VSTree *vstree;
		KVstore *kvstore;
		StringIndex *stringindex;
		Strategy strategy;
		QueryCache *query_cache;
		PathQueryHandler *pqHandler;
		TempResultSet *global_parent;
		int well_designed;

		TYPE_TRIPLE_NUM *pre2num;
		TYPE_TRIPLE_NUM *pre2sub;
		TYPE_TRIPLE_NUM *pre2obj;
		TYPE_PREDICATE_ID limitID_predicate;
		TYPE_ENTITY_LITERAL_ID limitID_literal;
		TYPE_ENTITY_LITERAL_ID limitID_entity;
		CSR *csr;
		shared_ptr<Transaction> txn;
    public:
    	FILE* fp;
    	bool export_flag;

	public:
		GeneralEvaluation(VSTree *_vstree, KVstore *_kvstore, StringIndex *_stringindex, QueryCache *_query_cache, \
			TYPE_TRIPLE_NUM *_pre2num,TYPE_TRIPLE_NUM *_pre2sub, TYPE_TRIPLE_NUM *_pre2obj, \
			TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal, \
			TYPE_ENTITY_LITERAL_ID _limitID_entity, CSR *_csr, shared_ptr<Transaction> txn = nullptr);
        GeneralEvaluation(const GeneralEvaluation& _ge): query_parser(_ge.query_parser), vstree(_ge.vstree), kvstore(_ge.kvstore), stringindex(_ge.stringindex), strategy(_ge.strategy), query_cache(_ge.query_cache), pqHandler(_ge.pqHandler), csr(_ge.csr), pre2num(_ge.pre2num), pre2sub(_ge.pre2sub), pre2obj(_ge.pre2obj), limitID_predicate(_ge.limitID_predicate), limitID_literal(_ge.limitID_literal), limitID_entity(_ge.limitID_entity), txn(_ge.txn), fp(_ge.fp), export_flag(_ge.export_flag), global_parent(0), well_designed(-1){}

		~GeneralEvaluation();

		bool parseQuery(const std::string &_query);
		QueryTree& getQueryTree();

		bool doQuery();

		void setStringIndexPointer(StringIndex* _tmpsi);
		
	private:
		TempResultSet *temp_result;

		struct EvaluationStackStruct
		{
			QueryTree::GroupPattern group_pattern;
			TempResultSet *result;
		};
		std::vector<EvaluationStackStruct> rewriting_evaluation_stack;

	public:
		bool expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &group_pattern, std::deque<QueryTree::GroupPattern> &queue);
		//TempResultSet* rewritingBasedQueryEvaluation(int dep);
		TempResultSet* queryEvaluation(int dep, TempResultSet* parent=0, TempResultSet* parent_alternative=0);

		void getFinalResult(ResultSet &ret_result);
		void releaseResult();

		void prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, TYPE_TRIPLE_NUM &update_triple_num);

	private:
		void loadCSR();
		void prepPathQuery();
		void pathVec2JSON(int src, int dst, const std::vector<int> &v, std::stringstream &ss);

		int constructTriplePattern(QueryTree::GroupPattern& triple_pattern, int dep);
		void getUsefulVarset(Varset& useful, int dep);
		bool checkBasicQueryCache(vector<QueryTree::GroupPattern::Pattern>& basic_query, TempResultSet *&sub_result, Varset& useful);
		void fillCandList(SPARQLquery& sparql_query, int dep, vector<vector<string> >& encode_varset);
		//void joinBasicQueryResult(SPARQLquery& sparql_query, TempResultSet *new_result, TempResultSet *sub_result, vector<vector<string> >& encode_varset, \
			vector<vector<QueryTree::GroupPattern::Pattern> >& basic_query_handle, long tv_begin, long tv_handle, int dep);
};
#endif // _QUERY_GENERALEVALUATION_H

