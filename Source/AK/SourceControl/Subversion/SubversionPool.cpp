/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6636
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#include "stdafx.h"

#include "SubversionPool.h"

SvnPool::SvnPool( apr_pool_t* in_pParentPool )
{
	m_pPool = svn_pool_create( in_pParentPool );
}

SvnPool::~SvnPool()
{
	svn_pool_destroy( m_pPool );
}

void SvnPool::Clear()
{
	svn_pool_clear( m_pPool );
}

SvnPool::operator apr_pool_t*()
{
	return m_pPool;
}