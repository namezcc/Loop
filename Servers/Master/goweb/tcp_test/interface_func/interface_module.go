package interface_func

type EventCall func(interface{})

type Imodule interface {
	SendEvent(f EventCall, d interface{})
}
