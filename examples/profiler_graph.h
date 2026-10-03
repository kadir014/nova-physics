/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_EXAMPLE_PROFILER_GRAPH_H
#define NOVAPHYSICS_EXAMPLE_PROFILER_GRAPH_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define NV_PROFILER_HISTORY 120 // frames
#define NV_PROFILER_SERIES_COUNT 13 // graph categories
#define NV_PROFILER_SERIES_LAST_N_FRAMES_MEAN 15 // mean of last N frames for one category


/**
 * @brief Profiler graph state.
 * 
 * Zero-initialize once, one state per profiler.
 */
typedef struct {
    double ms[NV_PROFILER_SERIES_COUNT][NV_PROFILER_HISTORY];
    size_t count, next;
    double axis_ms;
    unsigned int low_frames;
} nvProfilerGraph;

typedef struct {
    const char *name;
    size_t offset;
    struct nk_color color;
} nvProfilerGraphSeries;


struct nk_rect nv_pg_intersect(struct nk_rect a, struct nk_rect b) {
    float x = fmaxf(a.x, b.x), y = fmaxf(a.y, b.y);
    float r = fminf(a.x + a.w, b.x + b.w);
    float d = fminf(a.y + a.h, b.y + b.h);
    return nk_rect(x, y, fmaxf(0, r - x), fmaxf(0, d - y));
}

void nv_pg_text(
    struct nk_command_buffer *out,
    const struct nk_user_font *font,
    struct nk_rect rect,
    const char *text,
    struct nk_color color
) {
    // Constrain long names and formatted values to their own row.
    struct nk_rect saved = out->clip;
    struct nk_rect clip = nv_pg_intersect(saved, rect);
    if (clip.w <= 0 || clip.h <= 0) return;
    nk_push_scissor(out, clip);
    nk_draw_text(out, rect, text, (int)strlen(text), font, nk_rgba(0, 0, 0, 0), color);
    nk_push_scissor(out, saved);
}

double nv_pg_nice_range(double peak) {
    // Five readable divisions with headroom
    double raw = fmax(0.1, peak * 1.25) / 5.0;
    double unit = pow(10.0, floor(log10(raw)));
    double n = raw / unit;
    double step = n <= 1 ? 1 : n <= 2 ? 2 : n <= 2.5 ? 2.5 : n <= 5 ? 5 : 10;
    return step * unit * 5.0;
}

size_t nv_pg_index(const nvProfilerGraph *g, size_t logical) {
    return (g->next + NV_PROFILER_HISTORY - g->count + logical) % NV_PROFILER_HISTORY;
}

void nv_pg_record(
    nvProfilerGraph *g,
    const nvProfiler *sample,
    const nvProfilerGraphSeries *series
) {
    size_t s, i;
    double peak = 0;

    for (s = 0; s < NV_PROFILER_SERIES_COUNT; ++s) {
        double seconds;
        // assumes tightly packed, uses offssetof!
        memcpy(&seconds, (const char *)sample + series[s].offset, sizeof seconds);

        // Invalid samples become gaps
        g->ms[s][g->next] =
            isfinite(seconds) && seconds >= 0 && seconds <= 1e9
            ? seconds * 1000.0
            : -1.0;
    }

    g->next = (g->next + 1) % NV_PROFILER_HISTORY;
    if (g->count < NV_PROFILER_HISTORY) ++g->count;
    for (s = 0; s < NV_PROFILER_SERIES_COUNT; ++s) {
        for (i = 0; i < g->count; ++i) {
            peak = fmax(peak, g->ms[s][nv_pg_index(g, i)]);
        }
    }

    // Expand immediately so spikes stay visible
    // Shrink only after 120 new samples below 60% of the current range
    double shrink_perc = 0.6;

    if (g->axis_ms <= 0 || peak > g->axis_ms) {
        g->axis_ms = nv_pg_nice_range(peak);
        g->low_frames = 0;
    }
    else if (peak < g->axis_ms * shrink_perc) {
        if (++g->low_frames >= 120) {
            double target = nv_pg_nice_range(peak);
            if (target < g->axis_ms) g->axis_ms = target;
            g->low_frames = 0;
        }
    }
    else g->low_frames = 0;
}

void nv_pg_chart(
    struct nk_context *ctx,
    const nvProfilerGraph *g,
    const nvProfilerGraphSeries *series,
    struct nk_rect bounds
) {
    const struct nk_user_font *font = ctx->style.font;
    struct nk_command_buffer *out = nk_window_get_canvas(ctx);
    struct nk_color muted = nk_rgb(158, 164, 178);
    struct nk_rect saved = out->clip, plot;

    double range = g->axis_ms > 0 ? g->axis_ms : 0.1;

    char text[96];
    float label_width;
    size_t s, i;
    int tick;

    snprintf(text, sizeof text, range < 1 ? "%.3f ms" : "%.2f ms", range);
    label_width = font->width(font->userdata, font->height, text, (int)strlen(text)) + 12;
    plot = nk_rect(bounds.x + label_width, bounds.y + font->height, bounds.w - label_width - 8, bounds.h - 3 * font->height - 12);

    nk_push_scissor(out, nv_pg_intersect(saved, bounds));
    if (plot.w < 40 || plot.h < 40) {
        nv_pg_text(out, font, bounds, "Enlarge window to show chart", muted);
        nk_push_scissor(out, saved);
        return;
    }

    nk_fill_rect(out, plot, 0, nk_rgb(18, 20, 26));
    for (tick = 0; tick <= 5; ++tick) {
        float y = plot.y + plot.h * (1 - tick / 5.0f);
        double value = range * tick / 5;
        nk_stroke_line(out, plot.x, y, plot.x + plot.w, y, 1, nk_rgb(47, 51, 62));
        snprintf(text, sizeof text, range < 1 ? "%.3f ms" : "%.2f ms", value);
        nv_pg_text(out, font, nk_rect(bounds.x, y - font->height / 2, label_width - 6, font->height), text, muted);
    }

    for (tick = 0; tick <= 4; ++tick) {
        float x = plot.x + plot.w * tick / 4;
        nk_stroke_line(out, x, plot.y, x, plot.y + plot.h, 1, nk_rgb(38, 42, 52));
    }
    snprintf(text, sizeof text, "%zu frames ago", (size_t)NV_PROFILER_HISTORY - 1);
    nv_pg_text(out, font, nk_rect(plot.x, plot.y + plot.h + 6, fmaxf(0, plot.w - 42), font->height), text, muted);
    nv_pg_text(out, font, nk_rect(plot.x + plot.w - 34, plot.y + plot.h + 6, 34, font->height), "now", muted);

    nk_push_scissor(out, nv_pg_intersect(nv_pg_intersect(saved, bounds), plot));

    // One polyline per valid run shares vertices between adjacent segments
    // Each command is bound to 256 points, if Nuklear is given too few of a budget, everything breaks!
    for (s = NV_PROFILER_SERIES_COUNT; s-- > 0;) {
        float points[256 * 2];
        int used = 0;
        for (i = 0; i < g->count; ++i) {
            double ms = g->ms[s][nv_pg_index(g, i)];
            if (ms < 0) {
                if (used > 1) nk_stroke_polyline(out, points, used, s == 0 ? 2.0f : 1.0f, series[s].color);
                used = 0;
                continue;
            }
            points[used * 2] =
                NV_PROFILER_HISTORY == 1
                ? plot.x + plot.w
                : plot.x + plot.w * (float)(NV_PROFILER_HISTORY - g->count + i) / (float)(NV_PROFILER_HISTORY > 1 ? NV_PROFILER_HISTORY - 1 : 1);

            points[used * 2 + 1] = plot.y + plot.h * (float)(1 - fmin(1, ms / range));

            ++used;
            if (used == 256) {
                nk_stroke_polyline(out, points, used, s == 0 ? 2.0f : 1.0f, series[s].color);
                points[0] = points[(used - 1) * 2];
                points[1] = points[(used - 1) * 2 + 1];
                used = 1;
            }
        }
        if (used > 1) nk_stroke_polyline(out, points, used, s == 0 ? 2.0f : 1.0f, series[s].color);
        else if (used == 1) nk_fill_circle(out, nk_rect(points[0] - 2, points[1] - 2, 4, 4), series[s].color);
    }

    nk_push_scissor(out, nv_pg_intersect(saved, bounds));
    if (!g->count) {
        nv_pg_text(out, font,nk_rect(plot.x + 12, plot.y + 12, plot.w - 24, font->height), "Waiting for profiler samples", muted);
    }
    nk_push_scissor(out, saved);
}

void nv_pg_legend(
    struct nk_context *ctx,
    const nvProfilerGraph *g,
    const nvProfilerGraphSeries *series
) {
    const struct nk_user_font *font = ctx->style.font;
    struct nk_color text_color = nk_rgb(222, 226, 235);
    struct nk_color muted = nk_rgb(158, 164, 178);

    size_t window =
        g->count < NV_PROFILER_SERIES_LAST_N_FRAMES_MEAN
        ? g->count
        : NV_PROFILER_SERIES_LAST_N_FRAMES_MEAN;
    size_t s, i;

    char text[96];

    for (s = 0; s < NV_PROFILER_SERIES_COUNT; ++s) {
        struct nk_rect row, track;
        struct nk_command_buffer *out;

        double total = 0, paired_total = 0, step_total = 0, percentage = 0;
        size_t valid = 0, paired = 0;
        float row_height = font->height * 2 + 18;

        for (i = g->count - window; i < g->count; ++i) {
            size_t index = nv_pg_index(g, i);
            double value = g->ms[s][index], step = g->ms[0][index];
            if (value >= 0) { total += value; ++valid; }
            if (value >= 0 && step > 0) {
                paired_total += value; step_total += step; ++paired;
            }
        }

        if (paired) percentage = paired_total / step_total * 100.0;

        nk_layout_row_dynamic(ctx, row_height, 1);

        if (nk_widget(&row, ctx) == NK_WIDGET_INVALID) continue;

        out = nk_window_get_canvas(ctx);
        nk_fill_rect(out, nk_rect(row.x, row.y + 4, 7, 7), 1, series[s].color);
        nv_pg_text(out, font, nk_rect(row.x + 13, row.y, row.w - 13, font->height), series[s].name, text_color);
        
        if (!valid) snprintf(text, sizeof text, "-- ms    --%%");
        else if (!paired) snprintf(text, sizeof text, "%.3f ms    --%%", total / valid);
        // need a better way to handle degenrate percentages
        else if (percentage > 9999.0) snprintf(text, sizeof text, "%.3f ms    >9999%%", total / valid);
        else snprintf(text, sizeof text, "%.3f ms    %.1f%%", total / valid, percentage);

        nv_pg_text(out, font, nk_rect(row.x + 13, row.y + font->height + 2, row.w - 13, font->height), text, muted);
        track = nk_rect(row.x + 13, row.y + 2 * font->height + 7, fmaxf(0, row.w - 17), 4);
        nk_fill_rect(out, track, 2, nk_rgb(47, 51, 62));
        track.w *= (float)fmin(1, percentage / 100);

        if (track.w > 0) nk_fill_rect(out, track, 2, series[s].color);
    }
}


/**
 * @brief Entry point for profiler graph widget.
 * 
 * Call once per UI frame.
 * 
 * @param ctx Nuklear UI context.
 * @param graph Profiler graph.
 * @param sample nvProfiler sample.
 * @param bounds Bounds of the graph widget.
 */
void nv_profiler_window(
    struct nk_context *ctx,
    nvProfilerGraph *graph,
    const nvProfiler *sample,
    struct nk_rect bounds
) {
    #define _NV_PG_SERIES(field, label, r, g, b) {label, offsetof(nvProfiler, field), {r, g, b, 255}}
    static const nvProfilerGraphSeries series[NV_PROFILER_SERIES_COUNT] = {
        _NV_PG_SERIES(step, "Step", 240, 240, 245),
        _NV_PG_SERIES(broadphase, "Broadphase", 80, 150, 255),
        _NV_PG_SERIES(broadphase_finalize, "Broadphase finalize", 80, 220, 255),
        _NV_PG_SERIES(bvh_build, "BVH build", 0, 200, 180),
        _NV_PG_SERIES(bvh_traverse, "BVH traverse", 30, 220, 100),
        _NV_PG_SERIES(narrowphase, "Narrowphase", 255, 80, 80),
        _NV_PG_SERIES(integrate_accelerations, "Integrate accelerations", 255, 170, 40),
        _NV_PG_SERIES(presolve, "Presolve", 255, 220, 70),
        _NV_PG_SERIES(warmstart, "Warmstart", 190, 220, 70),
        _NV_PG_SERIES(solve_velocities, "Solve velocities", 190, 90, 255),
        _NV_PG_SERIES(solve_positions, "Solve positions", 255, 80, 210),
        _NV_PG_SERIES(integrate_velocities, "Integrate velocities", 150, 100, 255),
        _NV_PG_SERIES(raycasts, "Raycasts", 255, 130, 170)
    };

    if (!graph) return;
    if (sample) nv_pg_record(graph, sample, series);
    if (!ctx) return;

    if (
        nk_begin(
            ctx,
            "Physics Profiler",
            bounds,
            NK_WINDOW_BORDER |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_SCALABLE |
            NK_WINDOW_MINIMIZABLE |
            NK_WINDOW_TITLE |
            NK_WINDOW_NO_SCROLLBAR
        )
    ) {
        struct nk_vec2 size = nk_window_get_content_region_size(ctx);
        const struct nk_user_font *font = ctx->style.font;

        // Integrate accelerations is the longest category, take it as the reference
        // but maybe find a more elegant solution? Loop over all series to find the longest one?
        float legend_width = font->width(font->userdata, font->height, "Integrate accelerations", 23) + 52;
        float height = fmaxf(1, size.y - ctx->style.window.spacing.y - 4);
        float widths[2];
        struct nk_rect chart;

        // Keep text legible in narrow windows
        if (size.x < legend_width + 180) {
            nk_layout_row_dynamic(ctx, height, 1);
            if (nk_group_begin(ctx, "profiler_legend", 0)) {
                nv_pg_legend(ctx, graph, series);
                nk_group_end(ctx);
            }
        }
        else {
            widths[1] = legend_width;
            widths[0] = size.x - legend_width - ctx->style.window.spacing.x;
            nk_layout_row(ctx, NK_STATIC, height, 2, widths);
            if (nk_widget(&chart, ctx) != NK_WIDGET_INVALID)
                nv_pg_chart(ctx, graph, series, chart);
            if (nk_group_begin(ctx, "profiler_legend", 0)) {
                nv_pg_legend(ctx, graph, series);
                nk_group_end(ctx);
            }
        }
    }
    nk_end(ctx);
}


#endif