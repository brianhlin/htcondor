/***************************************************************
 *
 * Copyright (C) 1990-2010, Redhat.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

#include "kvm.h"

using namespace std;
using namespace boost;
using namespace condor::vmu;

#define KVM_TYPE_NAME "kvm"

kvm::kvm()
{
    m_szSessionID = "qemu:///session";
    m_szVMType = KVM_TYPE_NAME;
}

kvm::~kvm()
{

}

shared_ptr<hypervisor> kvm::manufacture(boost::shared_ptr<hypv_config> & local_config)
{
    shared_ptr<hypervisor> pRet(new kvm());
    local_config.reset(new libvirt_config());
    local_config->m_VM_TYPE = KVM_TYPE_NAME;

    return (pRet);
}