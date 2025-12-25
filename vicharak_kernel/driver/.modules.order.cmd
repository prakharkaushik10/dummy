cmd_/home/prakhar/vicharak_kernel/driver/modules.order := {   echo /home/prakhar/vicharak_kernel/driver/prakhar_ch.ko; :; } | awk '!x[$$0]++' - > /home/prakhar/vicharak_kernel/driver/modules.order
