#ifndef __ADTREE_H__
#define __ADTREE_H__

#include <string>
#include <vector>
#include <map>

namespace adtree
{
    typedef unsigned long dimension_t;
    typedef unsigned long long count_t;
    typedef std::vector<std::string*> recordlist_t;
    typedef std::map<std::string,std::string> query_t;
    typedef std::vector<std::string> columns_t;
    
    struct Condition
    {
        dimension_t i;
        std::string value;
    };
    
    struct ResultRow
    {
        std::string *columns;
        count_t count;
        
        ResultRow(dimension_t i)
        {
            count = 0;
            columns = new std::string[i];
        }
        
        ~ResultRow()
        {
            delete [] columns;
        }
    };
    
    typedef std::vector<ResultRow*> rows_t;
    
    struct VaryNode;
    
    struct ADNode
    {
        dimension_t base;
        dimension_t num_children;
        count_t count;
        VaryNode **children;
        
        ADNode(dimension_t base, dimension_t num_dimensions, count_t);
        
        ~ADNode();
        
        VaryNode * get_child(dimension_t i);
        
        void set_child(dimension_t, VaryNode *);
    };
    
    struct VaryNode
    {
        std::string mcv;
        std::map<std::string,ADNode *> children;
        
        VaryNode() {};
        
        ~VaryNode();
        
        ADNode * get_child(std::string k);
        
        void set_child(std::string k, ADNode *);
    };
    
    struct ADTree
    {
        std::string* dimensions;
        dimension_t num_dimensions;
        ADNode *root;
        
        ADTree(std::string* dimensions, dimension_t num_dimensions, recordlist_t records);
        
        ~ADTree();
        
        ADNode * make_adnode(dimension_t i, recordlist_t records);
        
        VaryNode * make_varynode(dimension_t i, recordlist_t records);
        
        count_t count(query_t query);
        
        count_t count(ADNode *, std::vector<Condition*> conditions, dimension_t i);
        
        rows_t table(columns_t columns);
        
        rows_t table(ADNode *, std::vector<dimension_t> columns, dimension_t i);
        
        rows_t table(columns_t columns, query_t query);
        
        void delete_rows(rows_t);
    };
}

#endif