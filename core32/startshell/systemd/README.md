# Startup

Startup can be summarised as being:

lingmo-core.target
lingmo-workspace@.target
graphical-session.target

lingmo-workspace@ is the target explicitly activated.

## X11 and wayland

lingmo-workspace@ is a template file that ends with x11 or wayland. That will then require the correct kwin_
startup order can be different between the two.

## Wants & Order
Note that in systemd dependencies (wants/wantedby) counter-intuitively do not determine order.

lingmo-workspace wants graphical-session, meaning it will make it something started by it, but it also explicitly comes before graphical-session.

The order of events is:
lingmo-core does anything that adjusts environment variables
lingmo-workspace@ starts all runtime services
graphical-session is at a point where everything including runtime services are up

## Adding a new service

If it should only be used on plasma it should be wanted by lingmo-core or lingmo-workspace@.

That service is responsible for setting "After=lingmo-core.target" if we need envs set up.
