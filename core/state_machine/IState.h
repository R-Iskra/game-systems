#pragma once

#include "EntityContext.h"

class IState {
    public:
        virtual ~IState() = default;

        virtual void enter(EntityContext& ctx) = 0;
        virtual void update(EntityContext& ctx, float delta) = 0;
        virtual void exit(EntityContext& ctx) = 0;

        virtual IState* get_transition(EntityContext& ctx) = 0;

        virtual const char* get_name() const = 0;
};