#ifndef _MESSAGE_SERVICE_IMPL_H_
#define _MESSAGE_SERVICE_IMPL_H_

#include <iostream>
#include <vector>
#include <memory>

#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/inherit.hpp>

#include "MessageList.h"
#include "MessageService.h"
#include "MessageServiceBase.h"


template<typename T>
class SampleSender : virtual public Sender<T>
{
   public:
      SampleSender() {};

      virtual ~SampleSender() {};

      // Our implementation does not handle a delay currently
      virtual double sendImpl( const T& msg, double delay );
};

template<typename T>
class SampleReceiver : virtual public Receiver<T>
{
   private:
      typedef typename boost::function<void( double, const T& )> CallbackType;
      typedef typename std::vector<CallbackType> CallbackVector;
      CallbackVector listeners;

   public:
      SampleReceiver()
         : listeners()
      { }

      virtual ~SampleReceiver() {};

      virtual void listenImpl( CallbackType callback )
      {
         listeners.push_back( callback );
      }

      virtual void sendMessageToListeners( const T& msg )
      {
         double sim_time = 0.0;

         typename CallbackVector::const_iterator it;

         for( it = listeners.begin();
               it != listeners.end();
               ++it )
         {
            ( *it )( sim_time, msg );
         }
      }
};


namespace detail
{
   // Original logic used. Want to save for reference
//   template<typename Framework>
//   struct SampleSenders
//   {
//      private:
//         typedef typename Framework::Senders SenderList;
//         // typedef typename boost:mpl::copy_if<SenderList, use_cml_processing<arg<1> > >::type common_senders;
//      public:
//         typedef typename boost::mpl::inherit_linearly< SenderList,
//               boost::mpl::inherit< boost::mpl::arg<1>, SampleSender<boost::mpl::arg<2> > > >::type type;
//   };
//
//   template<typename Framework>
//   struct SampleReceivers
//   {
//      private:
//         typedef typename Framework::Receivers ReceiverList;
//         // typedef typename copy_if<ReceiverList, use_cml_processing<arg<1> > > >::type common_receivers;
//      public:
//         typedef typename boost::mpl::inherit_linearly< ReceiverList,
//               boost::mpl::inherit< boost::mpl::arg<1>, SampleReceiver< boost::mpl::arg<2> > > >::type type;
//   };

   template<typename SenderList>
   struct SampleSenders
   {
      public:
         typedef typename boost::mpl::inherit_linearly< SenderList,
                 boost::mpl::inherit< boost::mpl::arg<1>, SampleSender<boost::mpl::arg<2> > > >::type type;
   };

   template<typename ReceiverList>
   struct SampleReceivers
   {
      public:
         typedef typename boost::mpl::inherit_linearly< ReceiverList,
                 boost::mpl::inherit< boost::mpl::arg<1>, SampleReceiver< boost::mpl::arg<2> > > >::type type;
   };
}

/**
 * Class to define structures to send/receive messages
 *
 * Note: Original logic was to use "MessageService" as a template parameter
 * to SampleSenders and SampleReceiver and grab the boost::mpl::vector
 * from its public typedefs.  This is a problem since MessageServiceImpl only
 * knew it had "SampleSenders" or "SampleReceivers" but did not necessarily know
 * about each individual "SampleSender" or "SampleReceiver".  In turn, we could not
 * use "forwardMessageToListeners" as an in-between step for senders to
 * actually send their messages.  Using the template parameters directly
 * allowed MessageServiceImpl to call directly to the appropriate Receiver to
 * send a message
 */
template<typename SenderList, typename ReceiverList>
class MessageServiceImpl : public MessageService,
   public detail::SampleSenders<SenderList>::type,
   public detail::SampleReceivers<ReceiverList>::type
{
   public:
      MessageServiceImpl() {};

      virtual ~MessageServiceImpl() {};

      template<typename MessageType>
      void forwardMessageToListeners( const MessageType &msg )
      {
         // We will not go through the trouble of making sure the "MessageType"
         // is in the Sender/Receiver list since the base class "MessageServiceBase"
         // does that for us already
         SampleReceiver<MessageType>* ptr = this;
         ptr->sendMessageToListeners( msg );
      }
};

struct SampleMessageService : virtual MessageServiceImpl<SenderTypes, ReceiverTypes>
{
   public:
      SampleMessageService();
      ~SampleMessageService() {};
};

namespace
{
   SampleMessageService *helper;
}

SampleMessageService::SampleMessageService()
{
   helper = this;
}

template<typename T>
double SampleSender<T>::sendImpl( const T& msg, double )
{
   helper->forwardMessageToListeners( msg );

   // TODO go to SchedulingService to get SimTime
   return 0.0;
}

#endif /* _MESSAGE_SERVICE_IMPL_H_ */
