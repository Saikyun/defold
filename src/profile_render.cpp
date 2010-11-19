#include <string.h>

#include <dlib/dstrings.h>
#include <dlib/math.h>
#include <dlib/profile.h>

#include "profile_render.h"

namespace dmProfileRender
{
    struct Context
    {
        float    m_Y;
        uint32_t m_TextY;
        float    m_Barheight;
        float    m_Spacing;
        int32_t  m_Index;
        int32_t  m_SampleIndex;
        float    m_TicksPerSecond;
        float    m_FrameX;
        dmRender::HRenderContext m_RenderContext;
        dmRender::HFont m_Font;
    };

//    float *r, *g, *b; /* red, green, blue in [0,1] */
  //  float h, s, l;    /* hue in [0,360]; saturation, light in [0,1] */
    void hsl_to_rgb(float* r, float* g, float* b, float h, float s, float l)
    {
        float c = (1.0f - dmMath::Abs(2.0f * l - 1.0f)) * s;
        float hp = h / 60.0f;
        int hpi = (int)hp;
        float hpmod2 = (hpi % 2) + (hp - hpi);
        float x = c * (1.0f - dmMath::Abs(hpmod2 - 1.0f));
        switch (hpi)
        {
            case 0: *r = c; *g = x; *b = 0; break;
            case 1: *r = x; *g = c; *b = 0; break;
            case 2: *r = 0; *g = c; *b = x; break;
            case 3: *r = 0; *g = x; *b = c; break;
            case 4: *r = x; *g = 0; *b = c; break;
            case 5: *r = c; *g = 0; *b = x; break;
        }
        float m = l - 0.5f * c;
        *r += m;
        *g += m;
        *b += m;
    }

    void HslToRgb2(float h, float s, float l, float* c)
    {
        hsl_to_rgb(&c[0], &c[1], &c[2], h * 360, s, l);
    }

    const int g_Scope_x0 = 16;
    const int g_Scope_Time_x0 = 16 + 120;
    const int g_Scope_Count_x0 = 16 + 180;
    const int g_Sample_x0 = 250;
    const int g_Sample_Time_x0 = g_Sample_x0 + 260;
    const int g_Frame_x0 = g_Sample_Time_x0 + 65;
    const int g_TextSpacing = 20;

    void ProfileSampleCallback(void* context, const dmProfile::Sample* sample)
    {
        Context* c = (Context*) context;
        Matrix4 m = Matrix4::orthographic( -1, 1, 1, -1, 10, -10 );

        const float freq = 60.0f;

        //float y = c->m_Y + sample->m_Depth * c->m_Spacing;
        float y = c->m_Y + sample->m_Scope->m_Index * c->m_Spacing;

        float x = c->m_FrameX + (1.0f * freq * sample->m_Start) / c->m_TicksPerSecond;
        float w = (1.0f * freq * sample->m_Elapsed) / c->m_TicksPerSecond;

        float col[3];
        HslToRgb2( (sample->m_Scope->m_Index % 16) / 16.0f, 1.0f, 0.65f, col);

        dmRender::Square2d(c->m_RenderContext, x, y, x + w, y + c->m_Barheight, Vector4(col[0], col[1], col[2], 1));

        float e = sample->m_Elapsed / c->m_TicksPerSecond;
        if (e > 0.0002f)
        {
            int text_y = c->m_TextY + c->m_SampleIndex * g_TextSpacing;

            char buf[256];

            dmRender::DrawTextParams params;
            params.m_Text = buf;
            params.m_Y = text_y;
            params.m_FaceColor = Vectormath::Aos::Vector4(col[0], col[1], col[2], 1.0f);
            params.m_ShadowColor = Vectormath::Aos::Vector4(0.0f, 0.0f, 0.0f, 1.0f);

            DM_SNPRINTF(buf, sizeof(buf), "%s.%s", sample->m_Scope->m_Name, sample->m_Name);
            params.m_X = g_Sample_x0;
            dmRender::DrawText(c->m_RenderContext, c->m_Font, params);

            DM_SNPRINTF(buf, sizeof(buf), "%.1f", e * 1000.0f);
            params.m_X = g_Sample_Time_x0;
            dmRender::DrawText(c->m_RenderContext, c->m_Font, params);

            c->m_SampleIndex++;
        }
        c->m_Index++;
    }

    void ProfileScopeCallback(void* context, const dmProfile::Scope* scope)
    {
        Context* c = (Context*) context;
        Matrix4 m = Matrix4::orthographic( -1, 1, 1, -1, 10, -10 );

        int y = c->m_TextY + c->m_Index * g_TextSpacing;

        float col[3];
        HslToRgb2( (scope->m_Index % 16) / 16.0f, 1.0f, 0.65f, col);

        float e = scope->m_Elapsed / c->m_TicksPerSecond;

        //if (e > 0.0001f)
        {
            char buf[256];

            dmRender::DrawTextParams params;
            params.m_Text = buf;
            params.m_Y = y;
            params.m_FaceColor = Vectormath::Aos::Vector4(col[0], col[1], col[2], 1.0f);
            params.m_ShadowColor = Vectormath::Aos::Vector4(0.0f, 0.0f, 0.0f, 1.0f);

            DM_SNPRINTF(buf, sizeof(buf), "%s", scope->m_Name);
            params.m_X = g_Scope_x0;
            dmRender::DrawText(c->m_RenderContext, c->m_Font, params);

            DM_SNPRINTF(buf, sizeof(buf), "%.1f", e * 1000);
            params.m_X = g_Scope_Time_x0;
            dmRender::DrawText(c->m_RenderContext, c->m_Font, params);

            DM_SNPRINTF(buf, sizeof(buf), "%d", scope->m_Samples);
            params.m_X = g_Scope_Count_x0;
            dmRender::DrawText(c->m_RenderContext, c->m_Font, params);

            c->m_Index++;
        }
    }

    void Draw(dmRender::HRenderContext render_context, dmRender::HFont font, uint32_t width, uint32_t height)
    {
        Matrix4 m = Matrix4::orthographic(-1, 1, 1, -1, 1, -1);

        dmGraphics::HContext context = dmGraphics::GetContext();

        dmGraphics::SetBlendFunc(context, dmGraphics::BLEND_FACTOR_SRC_ALPHA, dmGraphics::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
        dmGraphics::DisableState(context, dmGraphics::DEPTH_TEST);
        dmGraphics::EnableState(context, dmGraphics::BLEND);

        dmRender::Square2d(render_context, -1.0f, -1.0f, 1.0f, 1.0f, Vector4(0.1f, 0.1f, 0.1f, 0.4f));

        int text_y0 = 50 - g_TextSpacing;

        char buffer[256];

        dmRender::DrawTextParams params;
        params.m_Text = buffer;
        params.m_Y = text_y0;
        params.m_FaceColor = Vectormath::Aos::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        params.m_ShadowColor = Vectormath::Aos::Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        DM_SNPRINTF(buffer, 256, "Frame: %.3f Max: %.3f", dmProfile::GetFrameTime(), dmProfile::GetMaxFrameTime());
        params.m_X = g_Scope_x0;
        dmRender::DrawText(render_context, font, params);

        text_y0 += g_TextSpacing;

        float frame_x0 = 2.0f * g_Frame_x0 / (float)width - 1.0f;
        dmRender::Square2d(render_context, frame_x0, -0.85f, 1.0f, 0.15f, Vector4(0.1f, 0.1f, 0.15f, 0.4f));

        params.m_Y = text_y0;

        params.m_Text = "Scopes:";
        params.m_X = g_Scope_x0;
        dmRender::DrawText(render_context, font, params);
        params.m_Text = "ms";
        params.m_X = g_Scope_Time_x0;
        dmRender::DrawText(render_context, font, params);
        params.m_Text = "#";
        params.m_X = g_Scope_Count_x0;
        dmRender::DrawText(render_context, font, params);
        params.m_Text = "Samples:";
        params.m_X = g_Sample_x0;
        dmRender::DrawText(render_context, font, params);
        params.m_Text = "ms";
        params.m_X = g_Sample_Time_x0;
        dmRender::DrawText(render_context, font, params);
        params.m_Text = "Frame:";
        params.m_X = g_Frame_x0;
        dmRender::DrawText(render_context, font, params);

        Context ctx;
        ctx.m_Y = -0.78f;
        ctx.m_TextY = text_y0 + g_TextSpacing;
        ctx.m_Barheight = 0.05f;
        ctx.m_Spacing = 0.074f;
        ctx.m_Index = 0;
        ctx.m_SampleIndex = 0;
        ctx.m_TicksPerSecond = dmProfile::GetTicksPerSecond();
        ctx.m_FrameX = frame_x0;
        ctx.m_RenderContext = render_context;
        ctx.m_Font = font;

        dmProfile::IterateSamples(&ctx, &ProfileSampleCallback);

        ctx.m_Index = 0;
        dmProfile::IterateScopes(&ctx, &ProfileScopeCallback);
    }
}
