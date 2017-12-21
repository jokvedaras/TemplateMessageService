#ifndef _MESSAGE_LIST_H_
#define _MESSAGE_LIST_H_

#include <boost/mpl/vector.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/end.hpp>

// forward declare all the messages we use
class MockMessage;

// NOTE: if you get an error on either of these due to increasing
//       the number of elements, it is most likely due because
//       you went above the limit of 50 elements per boost::mpl::vector
//       Make sure to add your element to the bottom-most
//       boost::mpl::vector

typedef boost::mpl::vector <
MockMessage
> SenderTypes1;

typedef boost::mpl::vector <
MockMessage
> ReceiverTypes1;

/* Use similiar logic if you go over the 50 element limit
typedef boost::mpl::insert_range<
      SenderTypes1, boost::mpl::end<SenderTypes1>::type,
      SenderTypes2 >::type SenderTypes;
*/

typedef SenderTypes1 SenderTypes;
typedef ReceiverTypes1 ReceiverTypes;


#endif /* _MESSAGE_LIST_H_ */
