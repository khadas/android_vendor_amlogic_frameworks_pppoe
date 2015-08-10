/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>

#include "IScreenMediaSourceClient.h"

namespace android {

enum {
    NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
	DATACALLBACK,
};

class BpScreenMediaSourceClient: public BpInterface<IScreenMediaSourceClient>
{
public:
    BpScreenMediaSourceClient(const sp<IBinder>& impl)
        : BpInterface<IScreenMediaSourceClient>(impl)
    {
    }

    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IScreenMediaSourceClient::getInterfaceDescriptor());
        data.writeInt32(msg);
        data.writeInt32(ext1);
        data.writeInt32(ext2);
        if (obj && obj->dataSize() > 0) {
            data.appendFrom(const_cast<Parcel *>(obj), 0, obj->dataSize());
        }
        remote()->transact(NOTIFY, data, &reply, IBinder::FLAG_ONEWAY);
    }

    virtual int dataCallback(const sp<IMemory>& buffer)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IScreenMediaSourceClient::getInterfaceDescriptor());

		    data.writeStrongBinder(buffer->asBinder());

        remote()->transact(DATACALLBACK, data, &reply, IBinder::FLAG_ONEWAY);

        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(ScreenMediaSourceClient, "android.media.IScreenMediaSourceClient");

// ----------------------------------------------------------------------

status_t BnScreenMediaSourceClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case NOTIFY: {
            CHECK_INTERFACE(IScreenMediaSourceClient, data, reply);
            int msg = data.readInt32();
            int ext1 = data.readInt32();
            int ext2 = data.readInt32();
            Parcel obj;
            if (data.dataAvail() > 0) {
                obj.appendFrom(const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }

            notify(msg, ext1, ext2, &obj);
            return NO_ERROR;
        } break;
        case DATACALLBACK: {
            CHECK_INTERFACE(IScreenMediaSourceClient, data, reply);
            sp<IMemory> buffer = interface_cast<IMemory>(data.readStrongBinder());
            reply->writeInt32(dataCallback(buffer));
            return NO_ERROR;
        } break;
        default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
