#include "adtree.h"

using namespace std;
using namespace adtree;

ADNode::ADNode(dimension_t base, dimension_t num_dimensions, count_t count)
{
    this->base = base;
    this->count = count;
    this->num_children = num_dimensions - base;
    children = new VaryNode*[num_children];
    
    for (dimension_t i=0; i<num_children; i++)
    {
        children[i] = NULL;
    }
}

ADNode::~ADNode()
{
    for (dimension_t i=0; i<num_children; i++)
    {
        if (children[i] != NULL)
        {
            delete children[i];
        }
    }
    
    delete [] children;
}


VaryNode * ADNode::get_child(dimension_t i)
{
    return children[i-base];
}

void ADNode::set_child(dimension_t i, VaryNode *vn)
{
    children[i-base] = vn;
}


VaryNode::~VaryNode()
{
    for (auto it = children.begin(); it != children.end(); it++)
    {
        delete it->second;
    }
}


ADNode * VaryNode::get_child(string k)
{
    auto p = children.find(k);
    
    if (p == children.end())
    {
        return NULL;
    }
    
    return p->second;
}


void VaryNode::set_child(string k, ADNode *ad)
{
    children[k] = ad;
}


ADTree::ADTree(std::string * dimensions, dimension_t num_dimensions, recordlist_t records)
{
    this->dimensions = dimensions;
    this->num_dimensions = num_dimensions;
    root = make_adnode(0, records);
}


ADTree::~ADTree()
{
    delete root;
}


ADNode * ADTree::make_adnode(dimension_t i, recordlist_t records)
{
    auto ad = new ADNode(i, num_dimensions, records.size());
    
    for (dimension_t j=i; j<num_dimensions; j++)
    {
        ad->set_child(j, make_varynode(j, records));
    }
    
    return ad;
}


VaryNode * ADTree::make_varynode(dimension_t i, recordlist_t records)
{
    map<string,recordlist_t*> records_by_value;
    
    for (auto it = records.begin(); it != records.end(); it++)
    {
        recordlist_t *list;
        std::string * r = *it;
        string k = r[i];
        
        auto p = records_by_value.find(k);
        
        if (p != records_by_value.end())
        {
            list = p->second;
        }
        else
        {
            list = new recordlist_t;
            records_by_value[k] = list;
        }
        
        list->push_back(r);
    }
    
    string mcv;
    count_t count = 0;
    
    for (auto p = records_by_value.begin(); p != records_by_value.end(); p++)
    {
        if (p->second->size() > count)
        {
            count = p->second->size();
            mcv = p->first;
        }
    }
    
    auto vn = new VaryNode;
    vn->mcv = mcv;
    
    for (auto p = records_by_value.begin(); p != records_by_value.end(); p++)
    {
        if (p->first != mcv)
        {
            vn->set_child(p->first, make_adnode(i+1, *(p->second)));
        }
        
        delete p->second;
    }
    
    return vn;
}


count_t ADTree::count(query_t query)
{
    vector<Condition*> conditions;
    
    for (dimension_t i=0; i<num_dimensions; i++)
    {
        auto p = query.find(dimensions[i]);
        
        if (p != query.end())
        {
            auto condition = new Condition;
            condition->i = i;
            condition->value = p->second;
            conditions.push_back(condition);
        }
    }
    
    if (conditions.size() == 0)
    {
        return root->count;
    }
    
    count_t c = count(root, conditions, 0);
    
    for (auto it = conditions.begin(); it != conditions.end(); it++)
    {
        delete (*it);
    }
    
    return c;
}

count_t ADTree::count(ADNode *ad, std::vector<Condition*> conditions, dimension_t i)
{
    if (i == conditions.size())
    {
        return ad->count;
    }
    
    auto vn = ad->get_child(conditions[i]->i);
    
    if (conditions[i]->value.compare(vn->mcv) == 0)
    {
        count_t c = count(ad, conditions, i+1);
        
        for (auto p = vn->children.begin(); p != vn->children.end(); p++)
        {
            auto _c = count(p->second, conditions, i+1);
            c -= _c;
        }
        
        return c;
    }
    
    auto cad = vn->get_child(conditions[i]->value);
    
    if (cad == NULL)
    {
        return 0;
    }
    
    return count(cad, conditions, i+1);
}


rows_t ADTree::table(columns_t columns)
{
    vector<dimension_t> columns_i;
    
    for (dimension_t i=0; i<num_dimensions; i++)
    {
        for (auto it = columns.begin(); it != columns.end(); it++)
        {
            if (dimensions[i].compare(*it) == 0)
            {
                columns_i.push_back(i);
            }
        }
    }
    
    if (columns_i.size() == 0)
    {
        rows_t rows;
        return rows;
    }
    
    return table(root, columns_i, 0);
}

rows_t ADTree::table(ADNode *ad, vector<dimension_t> columns, dimension_t i)
{
    rows_t rows;
    
    if (i == columns.size())
    {
        auto row = new ResultRow(columns.size());
        row->count = ad->count;
        rows.push_back(row);
        return rows;
    }
    
    auto vn = ad->get_child(columns[i]);
    count_t total = 0;
    
    for (auto p = vn->children.begin(); p != vn->children.end(); p++)
    {
        auto next_rows = table(p->second, columns, i+1);
        
        for (auto it = next_rows.begin(); it != next_rows.end(); it++)
        {
            auto row = *it;
            total += row->count;
            row->columns[i] = p->first;
            rows.push_back(row);
        }
    }
    
    rows_t next_rows = table(ad, columns, i+1);
    
    if (columns.size()-1 != i)
    {
        for (auto it_a = next_rows.begin(); it_a != next_rows.end(); it_a++)
        {
            auto m = *it_a;
            
            for (auto it_b = rows.begin(); it_b != rows.end(); it_b++)
            {
                auto s = *it_b;
                bool match = true;
                
                for (dimension_t j=i+1; j < columns.size(); j++)
                {
                    if (m->columns[j].compare(s->columns[j]) != 0)
                    {
                        match = false;
                        break;
                    }
                }
                
                if (match)
                {
                    m->count -= s->count;
                }
            }
            
            if (m->count != 0)
            {
                m->columns[i] = vn->mcv;
                rows.push_back(m);
            }
        }
    }
    else
    {
        for (auto it = next_rows.begin(); it != next_rows.end(); it++)
        {
            auto row = *it;
            row->count -= total;
            
            if (row->count != 0)
            {
                row->columns[i] = vn->mcv;
                rows.push_back(row);
            }
        }
    }
    
    return rows;
}

void ADTree::delete_rows(rows_t rows)
{
    for (auto it = rows.begin(); it != rows.end(); it++)
    {
        delete [] (*it)->columns;
    }
}