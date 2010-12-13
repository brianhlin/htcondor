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

#ifndef VMGAHP_HYPERVISOR
#define VMGAHP_HYPERVISOR

#include <boost/shared_ptr.hpp>

namespace condor
{

    namespace vmu
    {
    /* types ? */

    /**
     * The following is an abstract base class for a hypervisor
     *
     *
     * @author Timothy St. Clair
     */
    class hypervisor
    {
         hypervisor(){;}
         virtual ~hypervisor(){;};

        static bool discover( std::list<some_type> & found_hypervisors );
        static boost::shared_ptr<hypervisor> manufacture (some_type & );


         virtual bool init(/*Some params*/)=0;

         virtual bool getStats( /*Some struct*/ )=0;

         virtual void getLastError(std::string & szError);

    private:

        virtual check_capabilities();

        std::string m_szLastError;
    };


    }

}

#endif