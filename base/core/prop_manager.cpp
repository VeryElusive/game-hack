#include "prop_manager.h"

// used: working path
#include "config.h"
// used: client interface
#include "interfaces.h"

PropManager::~PropManager( )
{
	this->Destroy( );
}

bool PropManager::Create( ) {
	auto entry = Interfaces::Client->GetAllClasses( );

	if ( !entry )
		return false;

	while ( entry ) {
		auto table = entry->pRecvTable;

		if ( table )
			this->m_pTables.emplace_back( table );

		entry = entry->pNext;
	}

	if ( this->m_pTables.empty( ) )
		return false;

	return true;
}

void PropManager::Destroy( ) {
	this->m_pTables.clear( );
}

int PropManager::GetOffset( const std::string& table_name, const std::string& prop_name ) {
	return ( this->GetProp( table_name, prop_name ) );
}

RecvVarProxyFn PropManager::Hook( const RecvVarProxyFn hooked, const std::string& table_name, const std::string& prop_name ) {
	RecvProp_t* prop = nullptr;

	if ( !GetProp( table_name, prop_name, &prop ) )
		return nullptr;

	auto restore = prop->oProxyFn;

	prop->oProxyFn = hooked;

	return restore;
}

int PropManager::GetProp( const std::string& table_name, const std::string& prop_name, RecvProp_t** out_prop ) {
	return ( this->GetProp( this->GetTable( table_name ), prop_name, out_prop ) );
}

int PropManager::GetProp( RecvTable_t* table, const std::string& prop_name, RecvProp_t** out_prop ) {
	int extra = 0;

	for ( int i = 0; i < table->nProps; i++ ) {
		auto prop = &table->pProps[ i ];
		auto child = prop->pDataTable;

		if ( child && child->nProps ) {
			int add = GetProp( child, prop_name, out_prop );

			if ( add )
				extra += ( prop->iOffset + add );
		}

		if ( prop_name.compare( prop->szVarName ) == 0 ) {
			if ( out_prop )
				*out_prop = prop;

			return ( prop->iOffset + extra );
		}
	}

	return extra;
}

RecvTable_t* PropManager::GetTable( const std::string& name ) {
	if ( !this->m_pTables.empty( ) ) {
		for ( auto& table : m_pTables ) {
			if ( name.compare( table->szNetTableName ) == 0 )
				return table;
		}
	}

	return nullptr;
}