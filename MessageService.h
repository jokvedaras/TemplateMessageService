#ifndef _MESSAGE_SERVICE_H_
#define _MESSAGE_SERVICE_H_

#include "MessageServiceBase.h"
#include "MessageList.h"

class MessageService : public MessageServiceBase<SenderTypes, ReceiverTypes>
{
   public:
      MessageService();
      virtual ~MessageService();
};

#endif /* _MESSAGE_SERVICE_H_ */
