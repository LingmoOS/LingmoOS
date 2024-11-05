#include "validatestate.h"
#include <QSharedPointer>
namespace KInstaller {

Validator::Validator(ValidateStateId state):m_state(state)
{

}

Validator::Validator(ValidateStateId state, const Partition::Ptr& partition)
    :m_partition(partition), m_state(state)
{

}

ValidateStateId Validator::state() const
{
    return m_state;
}

const Partition::Ptr Validator::partition() const
{
    return m_partition;
}

bool Validator::equals(const Validator &validator) const
{
    if(state() != validator.state()) {
        return  false;
    }
    if(partition().isNull() && validator.partition().isNull()) {
        return true;
    } else if(!partition().isNull() && !validator.partition().isNull()) {
        return partition()->m_partitionPath == validator.partition()->m_partitionPath
                && partition()->m_devPath == validator.partition()->m_devPath;
    } else {
        return  false;
    }

}

ValidateState::ValidateState()
    :Validator::Ptr(nullptr)
{

}

ValidateState::ValidateState(Validator* p)
    :Validator::Ptr(p)
{

}

ValidateState::ValidateState(ValidateStateId state)
    :Validator::Ptr(new Validator(state))
{

}

ValidateState::ValidateState(ValidateStateId state, const Partition::Ptr& partition)
    :Validator::Ptr(new Validator(state, partition))
{

}

bool ValidateState::eqauls(const ValidateState &state) const
{
    if(isNull() || state.isNull()) {
        return false;
    }
    return false;//state->equals(*get());
}

ValidateState::operator ValidateStateId() const
{
    return false;//isNull() ? static_cast<ValidateStateId>(InvalidId):get()->state();
}

}
