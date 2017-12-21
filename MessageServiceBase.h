#ifndef _MESSAGE_SERVICE_BASE_H_
#define _MESSAGE_SERVICE_BASE_H_

#include <boost/function.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/contains.hpp>

/**
 * Base interface for sending messages of the specified type
 */
template<typename T>
class Sender
{
   public:
      virtual ~Sender() {};

      /**
       * Send a message of the type for this. Note, the delay parameter
       * might not be honored depending on the implementation
       *
       * @param msg Message to send
       * @param delay Delay before send
       * #return Time the message was sent
       */
      virtual double sendImpl( const T& msg, double delay ) = 0;
};

/**
 * Base interface for receiving messages of the requested type. This
 * interface provides the mechanism to register a callback that will
 * be executed upon message receipt
 */
template<typename T>
class Receiver
{
   public:
      /*
       * This is a boost::function, which can accept any functor that takes
       * a double and a constant reference to the message type
       */
      typedef boost::function<void( double, const T& )> CallbackType;
      virtual ~Receiver() {};

      /**
       * Registers a callback that listens for incoming messages
       * @param callback Callback functor which accepts a double
       * and const reference to the message type
       */
      virtual void listenImpl( CallbackType callback ) = 0;
};

namespace detail
{
   /*
    * Using arg<N> instead of _N below because of interactions with
    * the bind _N placeholder. Boost bind places them in an named namespace,
    * which hides the definitions we need below
    */

   /*
    * Metafunction which takes two arguments and inherits from the second
    * argument virtually, and the first directly. Meant to be used
    * with inherit linearly below
    */
   template<typename T1, typename T2>
   struct VirtualInherit : T1, virtual T2
   {

   };

   /*
    * Specialization where if T1 is boost::mpl::empty_base, only inherits
    * from the second
    */
   template<typename T2>
   struct VirtualInherit<boost::mpl::empty_base, T2>: virtual T2
   {

   };

   /*
    * Metafunction which defines a member typedef which inherits virtually
    * from Sender<T>, for each T in SenderList
    */
   template<typename SenderList>
   struct SenderBase
   {
      typedef typename boost::mpl::inherit_linearly< SenderList,
              VirtualInherit< boost::mpl::arg<1>, Sender< boost::mpl::arg<2> > > >::type type;
   };

   /*
    * Metafunction which defines a member typedef which inherits
    * virtually from Receiver<T>, for each T in ReceiverList
    */
   template<typename ReceiverList>
   struct ReceiverBase
   {
      typedef typename boost::mpl::inherit_linearly< ReceiverList,
              VirtualInherit< boost::mpl::arg<1>, Receiver< boost::mpl::arg<2> > > >::type type;
   };
}

/*
 * Class takes two type list (must be MPL sequences), SenderList
 * and ReceiverList. This class will inherit from the senders
 * and receiver specified
 *
 * This class provides public members which use the interfaces.
 * These public members are responsible for checking the inputs
 * to ensure they are valid and performing the overload resolution
 * to ensure the correct method for the message type is used
 */
template<typename SenderList, typename ReceiverList>
class MessageServiceBase :
   public detail::SenderBase<SenderList>::type,
   public detail::ReceiverBase<ReceiverList>::type
{
   public:
      MessageServiceBase() {};
      virtual ~MessageServiceBase() {};

      typedef SenderList Senders;
      typedef ReceiverList Receivers;

      /*
       * @brief Sends a message
       * @details Sends the specified message, with the requested delay.
       * The input message type must be one of the types in the SenderList.
       * The delay is a requested delay before sending the message but some
       * implementations may not honor it
       *
       * Typical usage will be
       *   MessageType msg
       *   io->send(msg)
       *
       * The compiler will determine MessageType automatically. If there
       * is an issue with this, it can be directly specified using the
       * following syntax
       *
       *   io->send<MessageType>(msg);
       *
       * @param msg Reference to the listener
       * @param delay Requested delay, if any
       * @return send time of the message
       */
      template<typename MessageType>
      double send( MessageType const& msg, double delay = 0.0 )
      {
         // double check the input type
         BOOST_STATIC_ASSERT_MSG( ( boost::mpl::contains<SenderList, MessageType>::value ),
                                  "send: Requested Message Type not on the SenderList" );

         // Copy to this pointer which ensures that this is derived unambigously to the
         // appropiate sender. Using this pointer means there is no overload resolution
         // (since the Sender<MessageType> only has the one send method)
         Sender<MessageType> *ptr = this;
         return ptr->sendImpl( msg, delay );
      }

      /*
       * @brief Registers a callback to listen for a specific message type
       * @details Registers the specified callback functor to listen for a specific
       * message type. The input functor must meet the requirements for
       * Receiver<MessageType>::CallbackType (see above). MessageType must be one
       * of the types specified in the Receiver List
       *
       * Unlike send(), a call to listen must always specify the MessageType
       * manually
       *
       *   io->listen<Message>(func)
       *
       * @param func Functor (function object, pointer-to-function) that will be
       * called when the request message type is received
       */
      template<typename MessageType, typename F>
      void listen( F func )
      {
         BOOST_STATIC_ASSERT_MSG( ( boost::mpl::contains<ReceiverList, MessageType>::value ),
                                  "listen: Requested Message Type not on the ReceiverList" );

         // If this line fails, it's because the input function does not meet the
         // function signature. It must accept a double and a const MessageType&

         // See Receiver<T> for the definition
         typename Receiver<MessageType>::CallbackType cast_function( func );

         // We copy to this pointer, which ensures that this is derived
         // unambiguously to the appropriate receiver.  Using this pointer
         // means there is no overload resolution (since the Receiver<MessageType>
         // only has the one send method
         Receiver<MessageType>* ptr = this;
         ptr->listenImpl( cast_function );
      }

   private:
      MessageServiceBase( MessageServiceBase const& );
      void operator=( MessageServiceBase const& );
};


#endif /* _MESSAGE_SERVICE_BASE_H_ */
