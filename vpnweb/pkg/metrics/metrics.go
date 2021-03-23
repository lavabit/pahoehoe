package metrics

import (
	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promauto"
)

var (
	UnavailableLogins = promauto.NewCounter(prometheus.CounterOpts{
		Name: "vpnweb_unavailable_service_logins_total",
		Help: "The total number of failed logins (server error)",
	})

	FailedLogins = promauto.NewCounter(prometheus.CounterOpts{
		Name: "vpnweb_failed_logins_total",
		Help: "The total number of failed logins",
	})

	SuccessfulLogins = promauto.NewCounter(prometheus.CounterOpts{
		Name: "vpnweb_successful_logins_total",
		Help: "The total number of processed events",
	})

	DownloadedCerts = promauto.NewCounter(prometheus.CounterOpts{
		Name: "vpnweb_downloaded_certs_total",
		Help: "The total number of downloaded certs",
	})
)
