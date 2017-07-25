# ipvs_twos_choice

# Testing
    make
    sudo service keepalived stop
    sudo rmmod ip_vs_twos
    sudo insmod ip_vs_twos.ko
    sudo service keepalived start
    wrk -t24 -c500 -d30s <url>
    sudo ipvsadm -Ln
