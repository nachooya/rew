/**
 * @file re_ice.h  Interface to Interactive Connectivity Establishment (ICE)
 *
 * Copyright (C) 2010 Creytiv.com
 */


/** Defines the ICE Candidate-pair state */
enum ice_candpair_state {
	ICE_CANDPAIR_FROZEN = 0, /**< Frozen state (default)                 */
	ICE_CANDPAIR_WAITING=1,  /**< Waiting to become highest on list      */
	ICE_CANDPAIR_INPROGRESS, /**< In-Progress state;transaction in progr.*/
	ICE_CANDPAIR_SUCCEEDED,  /**< Succeeded state; successful check      */
	ICE_CANDPAIR_FAILED      /**< Failed state; check failed             */
};

/** ICE Configuration */
struct trice_conf {
	bool debug;             /**< Enable ICE debugging                  */
	bool trace;             /**< Enable tracing of Connectivity checks */
	bool ansi;              /**< Enable ANSI colors for debug output   */
};

struct trice;
struct ice_lcand;
struct ice_candpair;
struct stun_conf;


typedef bool (ice_cand_recv_h)(struct ice_lcand *lcand,
			       int proto, void *sock, const struct sa *src,
			       struct mbuf *mb, void *arg);


/** Local candidate */
struct ice_lcand {
	struct ice_cand_attr attr;   /**< Base class (inheritance)           */
	struct le le;                /**< List element                       */

	/* Base-address only set for SRFLX, PRFLX, RELAY */
	struct sa base_addr;    /* IP-address of "base" candidate (optional) */

	struct udp_sock *us;
	struct udp_helper *uh;
	struct tcp_sock *ts;    /* TCP for simultaneous-open or passive. */
	char ifname[32];        /**< Network interface, for diagnostics */
	int layer;
	ice_cand_recv_h *recvh;
	void *arg;

	// todo: remove
	struct trice *icem;           /* parent */

	struct {
		size_t n_tx;
		size_t n_rx;
	} stats;
};

/** Remote candidate */
struct ice_rcand {
	struct ice_cand_attr attr;   /**< Base class (inheritance)           */
	struct le le;                /**< List element                       */
};


/** Defines a candidate pair */
struct ice_candpair {
	struct le le;                /**< List element                       */
	struct ice_lcand *lcand;     /**< Local candidate                    */
	struct ice_rcand *rcand;     /**< Remote candidate                   */
	enum ice_candpair_state state;/**< Candidate pair state              */
	uint64_t pprio;              /**< Pair priority                      */
      //bool def;                    /**< Default flag                       */
	bool valid;                  /**< Valid flag                         */
	bool nominated;              /**< Nominated flag                     */
	bool estab;
	bool trigged;
	int err;                     /**< Saved error code, if failed        */
	uint16_t scode;              /**< Saved STUN code, if failed         */

	struct tcp_conn *tc;

	struct ice_tcpconn *conn;    /* the TCP-connection used */
};


typedef void (trice_estab_h)(struct ice_candpair *pair,
			     const struct stun_msg *msg, void *arg);


typedef void (trice_failed_h)(int err, uint16_t scode,
			    struct ice_candpair *pair, void *arg);


int  trice_alloc(struct trice **icemp, const struct trice_conf *conf,
		 bool controlling, const char *lufrag, const char *lpwd);
int  trice_set_remote_ufrag(struct trice *icem, const char *rufrag);
int  trice_set_remote_pwd(struct trice *icem, const char *rpwd);
int  trice_set_software(struct trice *icem, const char *sw);
void trice_set_controlling(struct trice *trice, bool controlling);
bool trice_is_controlling(const struct trice *icem);
int  trice_debug(struct re_printf *pf, const struct trice *icem);
struct trice_conf *trice_conf(struct trice *icem);


/* Candidates (common) */
int  trice_cand_print(struct re_printf *pf, const struct ice_cand_attr *cand);
enum ice_tcptype   ice_tcptype_reverse(enum ice_tcptype type);
const char        *ice_tcptype_name(enum ice_tcptype tcptype);
enum ice_cand_type ice_cand_type_base(enum ice_cand_type type);


/* Local candidates */
int trice_lcand_add(struct ice_lcand **lcandp, struct trice *icem,
		    unsigned compid, int proto, uint32_t prio,
		    const struct sa *addr, const struct sa *base_addr,
		    enum ice_cand_type type, const struct sa *rel_addr,
		    enum ice_tcptype tcptype,
		    void *sock, int layer);
struct list      *trice_lcandl(const struct trice *icem);
struct ice_lcand *trice_lcand_find(struct trice *icem,
				   enum ice_cand_type type,
				   unsigned compid, int proto,
				   const struct sa *addr);
struct ice_lcand *trice_lcand_find2(const struct trice *icem,
				    enum ice_cand_type type, int af);
void *trice_lcand_sock(struct trice *icem, const struct ice_lcand *lcand);
void trice_lcand_recv_packet(struct ice_lcand *lcand,
			     const struct sa *src, struct mbuf *mb);


/* Remote candidate */
struct list *trice_rcandl(const struct trice *icem);
int trice_rcand_add(struct ice_rcand **rcandp, struct trice *icem,
		    unsigned compid, const char *foundation, int proto,
		    uint32_t prio, const struct sa *addr,
		    enum ice_cand_type type, enum ice_tcptype tcptype);
struct ice_rcand *trice_rcand_find(struct trice *icem, unsigned compid,
				   int proto, const struct sa *addr);


/* ICE Candidate pairs */
struct list *trice_checkl(const struct trice *icem);
struct list *trice_validl(const struct trice *icem);
struct ice_candpair *trice_candpair_find_state(const struct list *lst,
					   enum ice_candpair_state state);
int  trice_candpair_debug(struct re_printf *pf, const struct ice_candpair *cp);
int  trice_candpairs_debug(struct re_printf *pf, bool ansi_output,
			   const struct list *list);


/* ICE checklist */
void trice_checklist_set_waiting(struct trice *icem);
int  trice_checklist_start(struct trice *icem, struct stun *stun,
			   uint32_t interval, bool use_cand,
			   trice_estab_h *estabh, trice_failed_h *failh,
			   void *arg);
bool trice_checklist_isrunning(const struct trice *icem);
bool trice_checklist_iscompleted(const struct trice *icem);


/* ICE Conncheck */
int trice_conncheck_send(struct trice *icem, struct ice_candpair *pair,
			bool use_cand);
