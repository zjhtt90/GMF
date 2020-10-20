/*
 * OstreamAppender.hh
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef _LOG4CPP_SYNCOSTREAMAPPENDER_HH
#define _LOG4CPP_SYNCOSTREAMAPPENDER_HH

#include <log4cpp/Portability.hh>
#include <string>
#include <iostream>
#include <log4cpp/LayoutAppender.hh>
#include <log4cpp/OstreamAppender.hh>

namespace log4cpp {

    /**
     * SyncOstreamAppender appends LoggingEvents to ostreams and flushes immediately.
     **/
    class LOG4CPP_EXPORT SyncOstreamAppender : public OstreamAppender {
        public:
    	SyncOstreamAppender(const std::string& name, std::ostream* stream);
        virtual ~SyncOstreamAppender();

        protected:
        virtual void _append(const LoggingEvent& event);
    };
}

#endif // _LOG4CPP_SYNCOSTREAMAPPENDER_HH
