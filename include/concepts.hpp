#pragma once

template<typename CoroutineType>
concept Concept_CorotineType = requires(CoroutineType ct){
    typename CoroutineType::promise_type;
    typename CoroutineType::packer_type;
    typename CoroutineType::value_type;
};

template<typename CoroutinePackerType>
concept Concept_CoroutinePackerType = requires(CoroutinePackerType ct){
    ct.get_id();
    ct.get_priority();
    typename CoroutinePackerType::co_base_t;
    typename CoroutinePackerType::co_base_handle_t;
};

template<typename Types>
concept Concept_CoroutinePackerType_or_CorotineType = requires(Types ct){
    typename Types::co_packer_com_type;
};

template<typename ...Types>
concept Concept_CorotineTypes = (Concept_CorotineType<Types>,...);

template<typename ...Types>
concept Concept_CoroutinePackerTypes = (Concept_CoroutinePackerType<Types>,...);

template<typename ...Types>
concept Concept_CoroutinePackerType_or_CorotineTypes = (Concept_CoroutinePackerType_or_CorotineType<Types>,...);

template<typename CoroutineType, template<typename> class  ContainerType>
concept Concept_ContainerType = requires(ContainerType<CoroutineType>){
    ContainerType<CoroutineType>::push_back(CoroutineType{});
    ContainerType<CoroutineType>::push_fornt(CoroutineType{});
    ContainerType<CoroutineType>::pop_back(CoroutineType{});
    ContainerType<CoroutineType>::pop_fornt(CoroutineType{});
};

namespace memmory_mode{
    struct LinearQueues{  LinearQueues(){}  };
};

namespace scheduler_mode{
    struct FairScheduling{  FairScheduling(){} };
    struct PriorityScheduling{ PriorityScheduling(){} };
};