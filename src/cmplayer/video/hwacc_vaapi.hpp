#ifndef HWACC_VAAPI_HPP
#define HWACC_VAAPI_HPP

#include "stdafx.hpp"
#include "hwacc.hpp"
#include "hwacc_helper.hpp"

#ifdef Q_OS_LINUX

#include <va/va.h>
#if VA_CHECK_VERSION(0, 34, 0)
//#define USE_VAVPP
#include <va/va_compat.h>
#else
static constexpr VAProfile VAProfileNone = (VAProfile)-1;
#endif

#ifdef USE_VAVPP
#include <va/va_vpp.h>
#endif

template<> struct HwAccX11Trait<IMGFMT_VAAPI> {
    using Profile = VAProfile;
    using Status = VAStatus;
    static constexpr Status success = VA_STATUS_SUCCESS;
    static constexpr const char *name = "VA-API";
    static const char *error(Status status) { return vaErrorStr(status); }
    using SurfaceID = VASurfaceID;
    static constexpr SurfaceID invalid = VA_INVALID_SURFACE;
    static auto destroySurface(SurfaceID id) -> void;
    static auto createSurfaces(int width, int height, int format, QVector<SurfaceID> &ids) -> bool;
};

using VaApiStatusChecker = HwAccX11StatusChecker<IMGFMT_VAAPI>;
using VaApiCodec = HwAccX11Codec<IMGFMT_VAAPI> ;
using VaApiSurface = HwAccX11Surface<IMGFMT_VAAPI>;
using VaApiSurfacePool = HwAccX11SurfacePool<IMGFMT_VAAPI>;

class HwAccVaApi : public HwAcc, public VaApiStatusChecker {
public:
    HwAccVaApi(AVCodecID codec);
    virtual ~HwAccVaApi();
    virtual auto isOk() const -> bool override;
    virtual auto context() const -> void* override;
    virtual auto getSurface() -> mp_image* override;
    virtual auto type() const -> Type override {return VaApiGLX;}
    virtual auto getImage(mp_image *mpi) -> mp_image*;
private:
    auto freeContext() -> void;
    auto fillContext(AVCodecContext *avctx, int w, int h) -> bool override;
private:
    struct Data;
    Data *d;
};

#ifdef USE_VAVPP
struct VaApiFilterCap {
    int algorithm = 0;
    VAProcFilterValueRange range = {0, 0, 0, 0};
};

struct VaApiFilterInfo : public VaApiStatusChecker {
    VaApiFilterInfo() {}
    VaApiFilterInfo(VAContextID context, VAProcFilterType type);
    auto type() const -> VAProcFilterType {return m_type;}
    const VaApiFilterCap *cap(int algorithm) const {
        for (const auto &cap : m_caps) {
            if (cap.algorithm == algorithm)
                return &cap;
        }
        return nullptr;
    }
    const QVector<int> &algorithms() const { return m_algorithms; }
    static auto description(VAProcFilterType type, int algorithm) -> QString;
    auto supports(int algorithm) const -> bool { return m_algorithms.contains(algorithm); }
private:
    QVector<int> m_algorithms;
    VAProcFilterType m_type = VAProcFilterNone;
    QVector<VaApiFilterCap> m_caps;
};
#endif

struct VaApi : public VaApiStatusChecker {
    static const VaApiCodec *codec(AVCodecID id) { return find(id, get().m_supported); }
    static auto glx() -> VADisplay {return m_display;}
#ifdef USE_VAVPP
    static auto toVAType(DeintMethod method) -> VAProcDeinterlacingType;
    static const VaApiFilterInfo *filter(VAProcFilterType type) { return find(type, get().m_filters); }
    static QList<VaApiFilterInfo> filters() { return get().m_filters.values(); }
    static QList<int> algorithms(VAProcFilterType type);
#endif
    static auto surfaceFormat() -> int {return get().m_surfaceFormat;}
    static auto toVAType(int mp_fields, bool first) -> int;
    static auto finalize() -> void;
    static auto initialize() -> void;
    static auto isAvailable() -> bool { return ok; }
private:
    auto setSurfaceFormat(int format) -> void { m_surfaceFormat = format; }
    auto hasEntryPoint(VAEntrypoint point, VAProfile profile = VAProfileNone) -> bool {
        auto entries = find(profile, m_entries); return entries && entries->contains(point);
    }
    template<class Map>
    static const typename Map::mapped_type *find(typename Map::key_type key, const Map &map) {
        const auto it = map.find(key); return (it != map.end()) ? &(*it) : nullptr;
    }
    auto initCodecs() -> void;
#ifdef USE_VAVPP
    auto initFilters() -> void;
    QMap<VAProcFilterType, VaApiFilterInfo> m_filters;
#endif
    static VaApi &get();
    VaApi();
    QVector<VAProfile> m_profiles;
    QMap<AVCodecID, VaApiCodec> m_supported;
    QMap<VAProfile, QVector<VAEntrypoint>> m_entries;
    int m_surfaceFormat = 0;
    static VADisplay m_display;
    static bool init, ok;
    friend class HwAccVaApi;
};

class VaApiMixer : public HwAccMixer, public VaApiStatusChecker{
public:
    ~VaApiMixer();
    auto create(const QList<OpenGLTexture2D> &textures) -> bool final;
    auto upload(const mp_image *mpi, bool deint) -> bool override;
    auto directRendering() const -> bool override { return true; }
    auto getAligned(const mp_image *mpi,
                    QVector<QSize> *bytes) -> mp_imgfmt final;
private:
    VaApiMixer(const QSize &size);

    void *m_glSurface = nullptr;
    friend class HwAcc;
};

#endif

#endif // HWACC_VAAPI_HPP
