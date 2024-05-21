#pragma once
#include <fstream>
#include <unordered_map>

#include "../havoc.h"
#include "../sdk/hash/fnv1a.h"
#include "../sdk/datatypes/datatable.h"
#include "../sdk/datatypes/datamap.h"

// wtf windows
#undef GetProp

class PropManager : public CSingleton<PropManager>
{
public:
	~PropManager( );

public:
	bool Create( );
	void Destroy( );

	int GetOffset( const std::string& table_name, const std::string& prop_name );
	RecvVarProxyFn Hook( const RecvVarProxyFn hooked, const std::string& table_name, const std::string& prop_name );

	int GetProp( const std::string& table_name, const std::string& prop_name, RecvProp_t** out_prop = nullptr );
	int GetProp( RecvTable_t* table, const std::string& prop_name, RecvProp_t** out_prop = nullptr );

	RecvTable_t* GetTable( const std::string& name );

private:
	std::vector<RecvTable_t*> m_pTables;
};